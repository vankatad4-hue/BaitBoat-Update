#pragma once
// R30H-95: dormant single-owner radio polling worker.
// Callback must be bounded, nonblocking, and must not access actuators or LVGL.
// Lifecycle calls require one exclusive owner outside the worker.
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <atomic>
#include <stdint.h>
namespace flytec_rtos {
class RadioWorker {
public:
 using Poll = void (*)(void*);
 RadioWorker() = default;
 RadioWorker(const RadioWorker&) = delete;
 RadioWorker& operator=(const RadioWorker&) = delete;
 bool begin(Poll poll, void* context) {
  if (!poll || running_.load() || !done_.load()) return false;
#if defined(CONFIG_FREERTOS_UNICORE) && CONFIG_FREERTOS_UNICORE
  (void)context;
  return false;
#else
  poll_=poll; context_=context;
  heartbeat_.store(0); wrongCore_.store(false);
  done_.store(false); running_.store(true);
  if (xTaskCreatePinnedToCore(&entry,"flytec_radio",4096,this,
       tskIDLE_PRIORITY+1,nullptr,0)!=pdPASS) {
   running_.store(false); done_.store(true); return false;
  }
  return true;
#endif
 }
 void stop() {
  running_.store(false);
  while (!done_.load()) vTaskDelay(1);
 }
 ~RadioWorker(){stop();}
 uint32_t heartbeat() const {return heartbeat_.load();}
 bool wrongCore() const {return wrongCore_.load();}
 bool running() const {return running_.load();}
private:
 static void entry(void* p){static_cast<RadioWorker*>(p)->run();}
 void run(){
  while(running_.load()){
   if(xPortGetCoreID()!=0){wrongCore_.store(true);running_.store(false);break;}
   poll_(context_);
   heartbeat_.fetch_add(1);
   vTaskDelay(1);
  }
  done_.store(true);
  vTaskDelete(nullptr);
 }
 Poll poll_=nullptr;
 void* context_=nullptr;
 std::atomic<bool> running_{false},done_{true},wrongCore_{false};
 std::atomic<uint32_t> heartbeat_{0};
};
}
