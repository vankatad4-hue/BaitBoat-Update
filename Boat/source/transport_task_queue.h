#pragma once
// R30H-62: FreeRTOS transport handoff. This header is intentionally dormant
// until a single radio producer and a single application consumer are assigned.
// STOP uses a dedicated length-one overwrite queue and is always read first.
// No heap allocation during send/receive; FreeRTOS queue creation allocates
// during startup. A failed initialization must deny the task migration.
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include <atomic>
namespace flytec_rtos {
template <typename Command, size_t Depth> class CommandHandoff {
 static_assert(Depth > 0, "Depth must be positive");
 static_assert(Depth <= 0xFFFFU, "Depth exceeds FreeRTOS queue limit");
 static_assert(std::is_trivially_copyable<Command>::value,
               "FreeRTOS byte-copy queue requires trivially copyable payload");
public:
 CommandHandoff() = default;
 CommandHandoff(const CommandHandoff&) = delete;
 CommandHandoff& operator=(const CommandHandoff&) = delete;
 bool begin() {
  // A partially initialized instance must not retain stale queue handles.
  if (ordinary_ || stop_ || mutex_) {
   if (ready()) return true;
   end();
  }
  mutex_ = xSemaphoreCreateMutex();
  ordinary_ = xQueueCreate(static_cast<UBaseType_t>(Depth), sizeof(Command));
  stop_ = xQueueCreate(1, sizeof(Command));
  if (!ready()) { end(); return false; }
  return true;
 }
 void end() {
  stopLatched_.store(true, std::memory_order_release);
  if (ordinary_) { vQueueDelete(ordinary_); ordinary_ = nullptr; }
  if (stop_) { vQueueDelete(stop_); stop_ = nullptr; }
  if (mutex_) { vSemaphoreDelete(mutex_); mutex_ = nullptr; }
 }
 ~CommandHandoff() { end(); }
 bool ready() const { return ordinary_ != nullptr && stop_ != nullptr && mutex_ != nullptr; }
 bool offer(const Command& cmd) {
  if (!ready() || stopLatched_.load(std::memory_order_acquire)) return false;
  if (xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  const bool ok = !stopLatched_.load(std::memory_order_acquire) &&
                  xQueueSend(ordinary_, &cmd, 0) == pdTRUE;
  xSemaphoreGive(mutex_);
  return ok;
 }
 // STOP admission invalidates pending ordinary commands before delivery.
 // An ordinary command already dequeued by the consumer still needs an
 // application-level STOP epoch check immediately before execution.
 bool offerStop(const Command& cmd) {
  if (!ready()) return false;
  stopLatched_.store(true, std::memory_order_release);
  // Fail closed if the short lock is busy; caller must retry STOP admission.
  if (xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  const bool ok = xQueueOverwrite(stop_, &cmd) == pdTRUE;
  if (ok) xQueueReset(ordinary_);
  xSemaphoreGive(mutex_);
  return ok;
 }
 // STOP is always selected before ordinary traffic. Consumers must still
 // enforce the authenticated STOP policy; queue priority is not authorization.
 bool receive(Command& cmd, bool& isStop) {
  isStop = false;
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  if (xQueueReceive(stop_, &cmd, 0) == pdTRUE) {
   isStop = true;
   xSemaphoreGive(mutex_);
   return true;
  }
  const bool ok = !stopLatched_.load(std::memory_order_acquire) &&
                  xQueueReceive(ordinary_, &cmd, 0) == pdTRUE;
  xSemaphoreGive(mutex_);
  return ok;
 }
 // Reinitialize on a new authenticated session to prevent stale commands.
 // Only the application STOP-clear policy may call this, never radio RX.
 void clearStopLatchAfterAuthorization() {
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return;
  xQueueReset(ordinary_);
  xQueueReset(stop_);
  stopLatched_.store(false, std::memory_order_release);
  xSemaphoreGive(mutex_);
 }
 void clear() {
  stopLatched_.store(true, std::memory_order_release);
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return;
  xQueueReset(ordinary_);
  xQueueReset(stop_);
  xSemaphoreGive(mutex_);
 }
 bool stopLatched() const { return stopLatched_.load(std::memory_order_acquire); }
private:
 std::atomic<bool> stopLatched_{false};
 SemaphoreHandle_t mutex_ = nullptr;
 QueueHandle_t ordinary_ = nullptr;
 QueueHandle_t stop_ = nullptr;
};
} // namespace flytec_rtos
