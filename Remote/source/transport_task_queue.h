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
  latchAndAdvance();
  if (ordinary_) { vQueueDelete(ordinary_); ordinary_ = nullptr; }
  if (stop_) { vQueueDelete(stop_); stop_ = nullptr; }
  if (mutex_) { vSemaphoreDelete(mutex_); mutex_ = nullptr; }
 }
 ~CommandHandoff() { end(); }
 bool ready() const { return ordinary_ && stop_ && mutex_; }
 bool offer(const Command& cmd) {
  if (!ready() || stopLatched()) return false;
  if (xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  Entry e{cmd, currentEpoch()};
  const bool ok = !stopLatched() && xQueueSend(ordinary_, &e, 0) == pdTRUE;
  xSemaphoreGive(mutex_);
  return ok;
 }
 // STOP latch is immediate even if the mutex is occupied. The STOP queue is
 // a separate FreeRTOS thread-safe single-slot overwrite: no mutex contention
 // can drop STOP admission. A caller retries if this returns false.
 bool offerStop(const Command& cmd) {
  // Only one STOP producer may write the overwrite slot at a time. A
  // competing producer gets false and must retry; the latch stays asserted.
  if (stopWriter_.test_and_set(std::memory_order_acquire)) return false;
  if (!ready()) { stopWriter_.clear(std::memory_order_release); return false; }
  const uint32_t newEpoch = latchAndAdvance();
  if (!stopLatched()) { stopWriter_.clear(std::memory_order_release); return false; }
  Entry e{cmd, newEpoch};
  const bool admitted = xQueueOverwrite(stop_, &e) == pdTRUE;
  if (xSemaphoreTake(mutex_, 0) == pdTRUE) {
   xQueueReset(ordinary_);
   xSemaphoreGive(mutex_);
  }
  stopWriter_.clear(std::memory_order_release);
  return admitted;
 }
 // epochToken must travel with the dequeued command to the execution gate.
 bool receive(Command& cmd, bool& isStop, uint32_t& epochToken) {
  isStop = false;
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  Entry e{};
  if (xQueueReceive(stop_, &e, 0) == pdTRUE) {
   cmd = e.command; epochToken = e.epoch; isStop = true;
   acknowledgedStopEpoch_.store(e.epoch, std::memory_order_release);
   xSemaphoreGive(mutex_);
   return true;
  }
  const bool ok = !stopLatched() && xQueueReceive(ordinary_, &e, 0) == pdTRUE &&
                  e.epoch == currentEpoch() && !stopLatched();
  if (ok) { cmd = e.command; epochToken = e.epoch; }
  xSemaphoreGive(mutex_);
  return ok;
 }
 // Legacy receive must not be used for actuator commands; it cannot carry
 // the epoch to the final execution boundary.
 bool receive(Command& cmd, bool& isStop) {
  // Never hand ordinary work to a consumer that cannot validate its epoch.
   if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  Entry e{};
  const bool gotStop = xQueueReceive(stop_, &e, 0) == pdTRUE;
  if (gotStop) { cmd = e.command; isStop = true;
   acknowledgedStopEpoch_.store(e.epoch, std::memory_order_release); }
  else isStop = false;
  xSemaphoreGive(mutex_);
   return gotStop;
 }
 bool mayExecute(uint32_t token) const {
  return ready() && !stopLatched() &&
         token == currentEpoch();
 }
 // The final authorization + command dispatch must occur inside this critical
 // section. Callback must be bounded/nonblocking and must not call this queue.
 // STOP can latch atomically while callback runs; the actuator owner must also
 // check stopLatched() before each hardware output and fail closed.
 template <typename Fn> bool executeIfCurrent(uint32_t token, Fn fn) {
  if (!ready() || stopLatched() || xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  const bool ok = !stopLatched() && token == currentEpoch();
  // Recheck after acquiring the lock; STOP can still latch while fn runs.
  // This is for non-actuating preparation only. Never execute GPIO/motors here.
  if (ok) fn();
  xSemaphoreGive(mutex_);
  return ok;
 }
 // Atomic STOP/rearm state CAS is the ordering point. A STOP arriving
 // after successful rearm immediately latches again; a STOP arriving before
 // rearm cannot be cleared until its generation has been dequeued.
 // Authorization is an external precondition; this method does not grant it.
 bool clearStopLatchAfterAuthorization() {
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return false;
  uint32_t observed = state_.load(std::memory_order_acquire);
  const uint32_t stopEpoch = observed >> 1;
  // Prevent generation wrap from revalidating an ancient token.
  if (stopEpoch >= 0x7FFFFFFEU) { xSemaphoreGive(mutex_); return false; }
  if ((observed & 1U) == 0U ||
      acknowledgedStopEpoch_.load(std::memory_order_acquire) != stopEpoch) {
   // Initial startup state is latched without an actual STOP request.
   if (observed != 1U) { xSemaphoreGive(mutex_); return false; }
  }
  Entry pending{};
  if (xQueueReceive(stop_, &pending, 0) == pdTRUE) {
   xQueueOverwrite(stop_, &pending);
   xSemaphoreGive(mutex_);
   return false;
  }
  // Increment epoch and clear latch in ONE CAS; concurrent STOP always wins
  // before or after this point, never between two independent stores.
  const uint32_t desired = ((stopEpoch + 1U) << 1);
  if (!state_.compare_exchange_strong(observed, desired,
          std::memory_order_acq_rel, std::memory_order_acquire)) {
   xSemaphoreGive(mutex_);
   return false;
  }
  xQueueReset(ordinary_);
  xSemaphoreGive(mutex_);
  return true;
 }
 void clear() {
  latchAndAdvance();
  if (!ready() || xSemaphoreTake(mutex_, 0) != pdTRUE) return;
  // Do not erase an authenticated STOP already admitted by another task.
  // clear() is a fail-closed cancellation, not a STOP acknowledgement.
  xQueueReset(ordinary_);
  xSemaphoreGive(mutex_);
 }
 bool stopLatched() const { return (state_.load(std::memory_order_acquire) & 1U) != 0U; }
 uint32_t currentEpoch() const { return state_.load(std::memory_order_acquire) >> 1; }
private:
 std::atomic_flag stopWriter_ = ATOMIC_FLAG_INIT;
 // bit 0: STOP latch; bits 1..31: generation. Never rearm on wrap.
 uint32_t latchAndAdvance() {
  uint32_t observed=state_.load(std::memory_order_acquire);
  for (;;) {
   const uint32_t oldEpoch=observed >> 1;
   // Exhausted generation space is permanently fail-closed until restart.
   if (oldEpoch >= 0x7FFFFFFEU) {
    // Saturate into STOP-latched state, never recycle old tokens.
    const uint32_t terminal=(0x7FFFFFFFU << 1) | 1U;
    if (state_.compare_exchange_weak(observed,terminal,
          std::memory_order_acq_rel,std::memory_order_acquire)) return 0x7FFFFFFFU;
    continue;
   }
   const uint32_t nextEpoch=oldEpoch + 1U;
   const uint32_t desired=(nextEpoch << 1) | 1U;
   if (state_.compare_exchange_weak(observed,desired,
          std::memory_order_acq_rel,std::memory_order_acquire))
    return nextEpoch;
  }
 }
 std::atomic<uint32_t> state_{1U};
 std::atomic<uint32_t> acknowledgedStopEpoch_{0U};
 SemaphoreHandle_t mutex_ = nullptr;
 QueueHandle_t ordinary_ = nullptr;
 QueueHandle_t stop_ = nullptr;
};
} // namespace flytec_rtos
