#pragma once
// R30H-87: opt-in, non-actuating ESP32-S3 dual-core scheduling diagnostic.
// No radio/GPIO/motor/solenoid access. Never invoke from an ISR.
// begin()/stop()/destructor require one exclusive lifecycle owner; no concurrent calls.
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
 bool begin() {
  if (running_.load(std::memory_order_acquire)) return true;
#if defined(CONFIG_FREERTOS_UNICORE) && CONFIG_FREERTOS_UNICORE
  return false;
#else
  // Never restart while a previous task is still exiting.
  if (!done0_.load(std::memory_order_acquire) ||
      !done1_.load(std::memory_order_acquire)) return false;
  core0Ticks_.store(0); core1Ticks_.store(0);
  wrong0_.store(0); wrong1_.store(0);
  done0_.store(false, std::memory_order_release);
  running_.store(true, std::memory_order_release);
  if (xTaskCreatePinnedToCore(&task0, "flytec_diag_c0", 3072, this,
                              tskIDLE_PRIORITY + 1, nullptr, 0) != pdPASS) {
   running_.store(false, std::memory_order_release);
   done0_.store(true, std::memory_order_release);
   return false;
  }
  done1_.store(false, std::memory_order_release);
  if (xTaskCreatePinnedToCore(&task1, "flytec_diag_c1", 3072, this,
                              tskIDLE_PRIORITY + 1, nullptr, 1) != pdPASS) {
   running_.store(false, std::memory_order_release);
   done1_.store(true, std::memory_order_release);
   waitForExit();
   return false;
  }
  return true;
#endif
 }
 Snapshot snapshot() const {
  return {core0Ticks_.load(), core1Ticks_.load(), wrong0_.load(),
          wrong1_.load(), running_.load(std::memory_order_acquire)};
 }
 // Blocking: only from a third task, not either diagnostic worker or ISR.
 // Wait for both workers to signal exit; do not free this object before stop.
 void stop() {
  running_.store(false, std::memory_order_release);
  waitForExit();
 }
 ~DualCoreDiagnostic() { stop(); }
private:
 static void task0(void* arg) { static_cast<DualCoreDiagnostic*>(arg)->run(0); }
 static void task1(void* arg) { static_cast<DualCoreDiagnostic*>(arg)->run(1); }
 void run(uint32_t expected) {
  while (running_.load(std::memory_order_acquire)) {
   if (static_cast<uint32_t>(xPortGetCoreID()) != expected)
    (expected == 0 ? wrong0_ : wrong1_).fetch_add(1);
   (expected == 0 ? core0Ticks_ : core1Ticks_).fetch_add(1);
   vTaskDelay(pdMS_TO_TICKS(100));
  }
  // Worker no longer dereferences this after publishing completion.
  (expected == 0 ? done0_ : done1_).store(true, std::memory_order_release);
  vTaskDelete(nullptr);
 }
 void waitForExit() {
  while (!done0_.load(std::memory_order_acquire) ||
         !done1_.load(std::memory_order_acquire))
   vTaskDelay(pdMS_TO_TICKS(1));
 }
 std::atomic<bool> running_{false};
 std::atomic<bool> done0_{true}, done1_{true};
 std::atomic<uint32_t> core0Ticks_{0}, core1Ticks_{0}, wrong0_{0}, wrong1_{0};
};
} // namespace flytec_rtos
