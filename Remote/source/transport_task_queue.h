#pragma once
// R30H-62: FreeRTOS transport handoff. This header is intentionally dormant
// until a single radio producer and a single application consumer are assigned.
// STOP uses a dedicated length-one overwrite queue and is always read first.
// No heap allocation during send/receive; FreeRTOS queue creation allocates
// during startup. A failed initialization must deny the task migration.
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdint.h>
namespace flytec_rtos {
template <typename Command, size_t Depth> class CommandHandoff {
 static_assert(Depth > 0, "Depth must be positive");
public:
 CommandHandoff() = default;
 CommandHandoff(const CommandHandoff&) = delete;
 CommandHandoff& operator=(const CommandHandoff&) = delete;
 bool begin() {
  if (ordinary_ || stop_) return ready();
  ordinary_ = xQueueCreate(static_cast<UBaseType_t>(Depth), sizeof(Command));
  stop_ = xQueueCreate(1, sizeof(Command));
  if (!ready()) { end(); return false; }
  return true;
 }
 void end() {
  if (ordinary_) { vQueueDelete(ordinary_); ordinary_ = nullptr; }
  if (stop_) { vQueueDelete(stop_); stop_ = nullptr; }
 }
 ~CommandHandoff() { end(); }
 bool ready() const { return ordinary_ != nullptr && stop_ != nullptr; }
 bool offer(const Command& cmd) {
  return ready() && xQueueSend(ordinary_, &cmd, 0) == pdTRUE;
 }
 bool offerStop(const Command& cmd) {
  return ready() && xQueueOverwrite(stop_, &cmd) == pdTRUE;
 }
 // STOP is always selected before ordinary traffic. Consumers must still
 // enforce the authenticated STOP policy; queue priority is not authorization.
 bool receive(Command& cmd, bool& isStop) {
  isStop = false;
  if (!ready()) return false;
  if (xQueueReceive(stop_, &cmd, 0) == pdTRUE) {
   isStop = true;
   return true;
  }
  return xQueueReceive(ordinary_, &cmd, 0) == pdTRUE;
 }
 // Reinitialize on a new authenticated session to prevent stale commands.
 void clear() {
  if (ordinary_) xQueueReset(ordinary_);
  if (stop_) xQueueReset(stop_);
 }
private:
 QueueHandle_t ordinary_ = nullptr;
 QueueHandle_t stop_ = nullptr;
};
} // namespace flytec_rtos
