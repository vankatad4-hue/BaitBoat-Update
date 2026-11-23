#pragma once
// R30H-70: dormant Core-0/Core-1 handoff; no radio task is enabled here.
// Only authenticated STOP may enter offerStop(). A successful queue write is
// NOT a Boat STOP ACK or physical motor-current-zero measurement.
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include <atomic>
namespace flytec_rtos {
template <typename Command, size_t Depth> class CommandHandoff {
 static_assert(Depth > 0 && Depth <= 0xFFFFU, "Invalid queue depth");
 static_assert(std::is_trivially_copyable<Command>::value, "Command must be trivially copyable");
 struct Entry { Command command; uint32_t epoch; };
public:
 CommandHandoff() = default;
 CommandHandoff(const CommandHandoff&) = delete;
 CommandHandoff& operator=(const CommandHandoff&) = delete;
 bool begin() {
  if (ordinary_ || stop_ || mutex_) { if (ready()) return true; end(); }
  mutex_ = xSemaphoreCreateMutex();
  ordinary_ = xQueueCreate(static_cast<UBaseType_t>(Depth), sizeof(Entry));
  stop_ = xQueueCreate(1, sizeof(Entry));
  if (!ready()) { end(); return false; }
  return true;
 }
 // Caller must first stop and join both worker tasks.
 void end() {
  stopLatched_.store(true, std::memory_order_release);
  epoch_.fetch_add(1, std::memory_order_acq_rel);
  if (ordinary_) { vQueueDelete(ordinary_); ordinary_ = nullptr; }
  if (stop_) { vQueueDelete(stop_); stop_ = nullptr; }
  if (mutex_) { vSemaphoreDelete(mutex_); mutex_ = nullptr; }
 }
 ~CommandHandoff() { end(); }
 bool ready() const { return ordinary_ && stop_ && mutex_; }
 bool offer(const Command& cmd) {
  if (!ready() || stopLatched()) return false;
  if (xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  Entry e{cmd, epoch_.load(std::memory_order_acquire)};
  const bool ok = !stopLatched() && xQueueSend(ordinary_, &e, 0) == pdTRUE;
  xSemaphoreGive(mutex_);
  return ok;
 }
 // STOP latch is immediate even if the mutex is occupied. The STOP queue is
 // a separate FreeRTOS thread-safe single-slot overwrite: no mutex contention
 // can drop STOP admission. A caller retries if this returns false.
 bool offerStop(const Command& cmd) {
  stopLatched_.store(true, std::memory_order_release);
  const uint32_t newEpoch = epoch_.fetch_add(1, std::memory_order_acq_rel) + 1U;
  if (!ready()) return false;
  Entry e{cmd, newEpoch};
  const bool admitted = xQueueOverwrite(stop_, &e) == pdTRUE;
  if (xSemaphoreTake(mutex_, 0) == pdTRUE) {
   xQueueReset(ordinary_);
   xSemaphoreGive(mutex_);
  }
  return admitted;
 }
 // epochToken must travel with the dequeued command to the execution gate.
 bool receive(Command& cmd, bool& isStop, uint32_t& epochToken) {
  isStop = false;
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  Entry e{};
  if (xQueueReceive(stop_, &e, 0) == pdTRUE) {
   cmd = e.command; epochToken = e.epoch; isStop = true;
   xSemaphoreGive(mutex_);
   return true;
  }
  const bool ok = !stopLatched() && xQueueReceive(ordinary_, &e, 0) == pdTRUE &&
                  e.epoch == epoch_.load(std::memory_order_acquire) && !stopLatched();
  if (ok) { cmd = e.command; epochToken = e.epoch; }
  xSemaphoreGive(mutex_);
  return ok;
 }
 // Legacy receive must not be used for actuator commands; it cannot carry
 // the epoch to the final execution boundary.
 bool receive(Command& cmd, bool& isStop) {
  uint32_t ignored = 0;
  return receive(cmd, isStop, ignored);
 }
 bool mayExecute(uint32_t token) const {
  return ready() && !stopLatched() &&
         token == epoch_.load(std::memory_order_acquire);
 }
 // The final authorization + command dispatch must occur inside this critical
 // section. Callback must be bounded/nonblocking and must not call this queue.
 // STOP can latch atomically while callback runs; the actuator owner must also
 // check stopLatched() before each hardware output and fail closed.
 template <typename Fn> bool executeIfCurrent(uint32_t token, Fn fn) {
  if (!ready() || stopLatched() || xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  const bool ok = !stopLatched() && token == epoch_.load(std::memory_order_acquire);
  if (ok) fn();
  xSemaphoreGive(mutex_);
  return ok;
 }
 bool clearStopLatchAfterAuthorization() {
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  xQueueReset(ordinary_); xQueueReset(stop_);
  epoch_.fetch_add(1, std::memory_order_acq_rel);
  stopLatched_.store(false, std::memory_order_release);
  xSemaphoreGive(mutex_);
  return true;
 }
 void clear() {
  stopLatched_.store(true, std::memory_order_release);
  epoch_.fetch_add(1, std::memory_order_acq_rel);
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return;
  xQueueReset(ordinary_); xQueueReset(stop_);
  xSemaphoreGive(mutex_);
 }
 bool stopLatched() const { return stopLatched_.load(std::memory_order_acquire); }
 uint32_t currentEpoch() const { return epoch_.load(std::memory_order_acquire); }
private:
 std::atomic<bool> stopLatched_{true};
 std::atomic<uint32_t> epoch_{0};
 SemaphoreHandle_t mutex_ = nullptr;
 QueueHandle_t ordinary_ = nullptr;
 QueueHandle_t stop_ = nullptr;
};
} // namespace flytec_rtos
