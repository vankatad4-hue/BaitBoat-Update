#pragma once
#if defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "r30h118_worker.h"
namespace flytec_r30h118 {
// Static lifetime only. One-shot mock; never accesses physical peripherals.
class MockTask {
 Transport<uint32_t> bus_;
 Worker<uint32_t> worker_{bus_};
 std::atomic<bool> started_{false},finished_{false};
 std::atomic<uint32_t> maxUs_{0},stackWords_{0},calls_{0};
 bool launched_=false,stopping_=false;
 static void entry(void *p){
  auto &s=*static_cast<MockTask*>(p);
  s.started_.store(true,std::memory_order_release);
  while(!s.worker_.exited()){
   const uint32_t begin=micros();
   s.worker_.step(millis(),100U,[&](uint32_t){s.calls_.fetch_add(1,std::memory_order_relaxed);return false;});
   const uint32_t elapsed=micros()-begin;
   if(elapsed>s.maxUs_.load(std::memory_order_relaxed))s.maxUs_.store(elapsed,std::memory_order_relaxed);
   s.stackWords_.store(uxTaskGetStackHighWaterMark(nullptr),std::memory_order_relaxed);
   vTaskDelay(1);
  }
  s.finished_.store(true,std::memory_order_release);
  vTaskDelete(nullptr);
 }
public:
 bool start(){
  if(launched_)return false;
  launched_=true;
  if(!bus_.invalidate(1))return false;
  if(xTaskCreatePinnedToCore(entry,"r118mock",4096,this,1,nullptr,0)!=pdPASS){bus_.invalidate(0);return false;}
  return true;
 }
 bool ready()const{return started_.load(std::memory_order_acquire)&&!finished();}
 bool submit(uint32_t v){return ready()&&!stopping_&&bus_.submit(v,millis(),false);}
 bool poll(bool &admitted){return bus_.poll(admitted);}
 void stop(){if(!stopping_){stopping_=true;worker_.requestStop();}}
 bool finished()const{return finished_.load(std::memory_order_acquire);}
 uint32_t maxStepUs()const{return maxUs_.load(std::memory_order_relaxed);}
 uint32_t stackHighWaterRaw()const{return stackWords_.load(std::memory_order_relaxed);}
 uint32_t mockCalls()const{return calls_.load(std::memory_order_relaxed);}
};
}
#endif
