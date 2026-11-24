#pragma once
#include "r30h120_physical_worker.h"
#include "r30h120_lifecycle.h"
#include <atomic>
namespace flytec_r30h120 {
// Not instantiated by production. Owner must retain this object and buses
// until the task has exited. Never start alongside the legacy radio owner.
class PhysicalTask {
 using Bus=flytec_r30h118::Transport<RadioFrame>;
 using Events=flytec_r30h118::PhyEvents<RadioFrame>;
public:
 struct Config {int ce,csn; SPIClass *spi; uint8_t channel; int sck,miso,mosi; E22BoardPins pins; bool primary,backup;};
private:
 Config cfg_;
 Bus &nb_,&eb_; Events &ne_,&ee_;
 Lifecycle life_;
 std::atomic<uint32_t> maxUs_{0},stackRaw_{0},cycles_{0};
 std::atomic<bool> exited_{false};
 static void entry(void *arg){static_cast<PhysicalTask*>(arg)->run();vTaskDelete(nullptr);}
 void run(){
  {
   PhysicalWorker w(cfg_.ce,cfg_.csn,cfg_.spi,cfg_.channel,cfg_.sck,cfg_.miso,cfg_.mosi,nb_,eb_,ne_,ee_);
   const bool ok=life_.phase()==Phase::Starting&&w.begin(true,cfg_.primary,cfg_.pins,cfg_.backup);
   if(ok&&life_.ready()){
    while(life_.enabled()){
     const uint32_t start=micros();
     w.step(millis(),100,life_.enabled());
     const uint32_t elapsed=micros()-start;
     if(elapsed>maxUs_.load())maxUs_.store(elapsed);
     stackRaw_.store(uxTaskGetStackHighWaterMark(nullptr));
     cycles_.fetch_add(1);
     vTaskDelay(1);
    }
   }
   w.shutdown();
   // Physical RF quiescence has not been established; fail closed.
   life_.finish(false);
  }
  exited_.store(true,std::memory_order_release);
 }
public:
 PhysicalTask(const Config &c,Bus &nb,Bus &eb,Events &ne,Events &ee):cfg_(c),nb_(nb),eb_(eb),ne_(ne),ee_(ee){}
 PhysicalTask(const PhysicalTask&)=delete;
 PhysicalTask& operator=(const PhysicalTask&)=delete;
 bool start(bool commissioningAuthorized,bool legacyOwnerAbsent){
  if(!commissioningAuthorized||!legacyOwnerAbsent||(!cfg_.primary&&!cfg_.backup)||!life_.start())return false;
  if(xTaskCreatePinnedToCore(entry,"boat_phy",6144,this,1,nullptr,0)!=pdPASS){life_.finish(false);exited_.store(true);return false;}
  return true;
 }
 void stop(){nb_.invalidate(0);eb_.invalidate(0);life_.stop();}
 Phase phase()const{return life_.phase();}
 bool exited()const{return exited_.load(std::memory_order_acquire);}
 uint32_t maxUs()const{return maxUs_.load();}
 uint32_t stackRaw()const{return stackRaw_.load();}
 uint32_t cycles()const{return cycles_.load();}
};
}
