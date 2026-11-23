#pragma once
// R30H-86: opt-in, non-actuating ESP32-S3 dual-core scheduling diagnostic.
// This module deliberately does NOT initialize radios, motor drivers, GPIO,
// command queues, watchdog feeding or any other physical output.
// Invoke begin() explicitly from a diagnostic-only entry point after boot.
// Never enable in a production control loop without hardware acceptance.
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <atomic>
#include <stdint.h>

namespace flytec_rtos {
class DualCoreDiagnostic {
public:
 struct Snapshot {
  uint32_t core0Ticks;
  uint32_t core1Ticks;
  uint32_t core0WrongCore;
  uint32_t core1WrongCore;
  bool running;
 };
 DualCoreDiagnostic() = default;
 DualCoreDiagnostic(const DualCoreDiagnostic&) = delete;
 DualCoreDiagnostic& operator=(const DualCoreDiagnostic&) = delete;
 // Call only from a single boot/diagnostic owner; not thread-safe with stop().
 bool begin() {
  if (running_.load(std::memory_order_acquire)) return true;
#if !defined(CONFIG_FREERTOS_UNICORE)
  core0Ticks_.store(0); core1Ticks_.store(0);
  wrong0_.store(0); wrong1_.store(0);
  running_.store(true, std::memory_order_release);
  if (xTaskCreatePinnedToCore(&task0, "flytec_diag_c0", 3072, this,
                              tskIDLE_PRIORITY + 1, &handle0_, 0) != pdPASS) {
   running_.store(false, std::memory_order_release);
   handle0_ = nullptr;
   return false;
  }
  if (xTaskCreatePinnedToCore(&task1, "flytec_diag_c1", 3072, this,
                              tskIDLE_PRIORITY + 1, &handle1_, 1) != pdPASS) {
   running_.store(false, std::memory_order_release);
   // Do not free the object while the first worker may still reference it.
   // Stop and join through task notifications before returning.
   xTaskNotifyGive(handle0_);
   while (handle0_ != nullptr) vTaskDelay(pdMS_TO_TICKS(1));
   return false;
  }
  return true;
#else
  return false; // Explicitly reject single-core FreeRTOS configurations.
#endif
 }
 Snapshot snapshot() const {
  return {core0Ticks_.load(), core1Ticks_.load(), wrong0_.load(),
          wrong1_.load(), running_.load(std::memory_order_acquire)};
 }
 // Call from a normal task, never an ISR. Waits for both tasks to exit.
 void stop() {
  running_.store(false, std::memory_order_release);
  if (handle0_) xTaskNotifyGive(handle0_);
  if (handle1_) xTaskNotifyGive(handle1_);
  while (handle0_ || handle1_) vTaskDelay(pdMS_TO_TICKS(1));
 }
 ~DualCoreDiagnostic() { stop(); }
private:
 static void task0(void* arg) { static_cast<DualCoreDiagnostic*>(arg)->run(0); }
 static void task1(void* arg) { static_cast<DualCoreDiagnostic*>(arg)->run(1); }
 void run(uint32_t expected) {
  while (running_.load(std::memory_order_acquire)) {
   if (static_cast<uint32_t>(xPortGetCoreID()) != expected) {
    (expected == 0 ? wrong0_ : wrong1_).fetch_add(1);
   }
   (expected == 0 ? core0Ticks_ : core1Ticks_).fetch_add(1);
   ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
  }
  // Clear handle before self-delete; owner must not access handle after stop.
  if (expected == 0) handle0_ = nullptr;
  else handle1_ = nullptr;
  vTaskDelete(nullptr);
 }
 std::atomic<bool> running_{false};
 std::atomic<uint32_t> core0Ticks_{0}, core1Ticks_{0}, wrong0_{0}, wrong1_{0};
 TaskHandle_t handle0_ = nullptr, handle1_ = nullptr;
};
} // namespace flytec_rtos
