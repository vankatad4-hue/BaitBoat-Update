#pragma once
// Consolidated Boat radio component; original types and owner contracts retained.
#include "nrf24_link.h"
#include "e22_uart_board_port.h"
#include "boat_radio_phy.h"
#include "boat_radio_worker_policy.h"
namespace flytec_r30h120 {
// Construct and call only inside the physical worker task.
// No task launch or hardware enable is implied by this class.
class PhysicalWorker {
 using Bus=flytec_r30h118::Transport<RadioFrame>;
 using Events=flytec_r30h118::PhyEvents<RadioFrame>;
 NRF24Link nrf_;
 HardwareSerial serial_{1};
 E22UartBoardPort e22_{serial_,UART_NUM_1};
 const flytec_rtos::LoRaAirtimeProfile e22Profile_;
 const uint32_t e22TailMs_;
 const bool e22AirtimeEvidenceSigned_;
 flytec_rtos::LoRaAirtimePolicy e22Airtime_;
 flytec_rtos::LoRaQuietRestart e22QuietRestart_;
 flytec_rtos::E22AdaptiveAdmission<E22UartBoardPort> e22Admission_;
 bool e22AirtimeConfigured_=false;
 flytec_r30h118::NrfAdapter<RadioFrame,NRF24Link> na_;
 flytec_r30h118::E22Adapter<RadioFrame,E22UartBoardPort> ea_;
 PhyService<RadioFrame> ns_,es_;
 // Optional commissioning-only shared services; legacy constructor remains unchanged.
 SharedPhyService<RadioFrame>* sharedNrf_=nullptr;
 SharedPhyService<RadioFrame>* sharedE22_=nullptr;
 uint8_t boatId_=0,remoteId_=0;
 bool nrfInitialized_=false,e22Initialized_=false,started_=false,closed_=false;
public:
 PhysicalWorker(int ce,int csn,SPIClass *spi,uint8_t channel,int sck,int miso,int mosi,
                Bus &nb,Bus &eb,Events &ne,Events &ee,
                const flytec_rtos::LoRaAirtimeProfile &profile,uint32_t tailMs,bool airtimeEvidenceSigned):
 nrf_(ce,csn,spi,channel,sck,miso,mosi),e22Profile_(profile),e22TailMs_(tailMs),
 e22AirtimeEvidenceSigned_(airtimeEvidenceSigned),
 e22Admission_(e22_,e22Airtime_,tailMs,airtimeEvidenceSigned&&profile.commissioned),
 na_(nrf_,ne),ea_(e22_,ee,e22Admission_),ns_(nb),es_(eb){}
 // Bind worker-local services before begin; their lifetime must cover this worker.
 bool bindShared(SharedPhyService<RadioFrame>& primary,SharedPhyService<RadioFrame>& backup,uint8_t boat,uint8_t remote){
  if(started_||closed_||sharedNrf_||sharedE22_||&primary==&backup||!boat||!remote||boat==remote)return false;
  sharedNrf_=&primary;sharedE22_=&backup;boatId_=boat;remoteId_=remote;return true;
 }
 PhysicalWorker(const PhysicalWorker&)=delete;
 PhysicalWorker& operator=(const PhysicalWorker&)=delete;
 // Caller must enforce existing board pin/RF commissioning gates first.
 bool begin(bool authorized,bool primary,const E22BoardPins &pins,bool backup){
  if(!authorized||started_||closed_)return false;
  started_=true;
  if(primary)nrfInitialized_=nrf_.begin(false);
  if(backup)e22Initialized_=e22_.begin(pins);
  return nrfInitialized_||e22Initialized_;
 }
 void step(uint32_t now,uint32_t age,bool enabled){
  if(!started_||closed_)return;
  if(nrfInitialized_)na_.poll(now);
  if(e22Initialized_)ea_.poll(now);
  if(e22Initialized_&&!e22AirtimeConfigured_){
   const bool quiet=e22_.configured()&&!e22_.txPending()&&!e22_.transportFault();
   e22QuietRestart_.observe(now,quiet);
   if(quiet&&e22AirtimeEvidenceSigned_&&e22Profile_.commissioned&&
      e22TailMs_>0U&&e22TailMs_<60000U&&e22QuietRestart_.ready(now))
    e22AirtimeConfigured_=e22Airtime_.configureAfterQuiet(e22Profile_,now,e22QuietRestart_);
  }
  if(sharedNrf_&&sharedE22_){
   auto primaryAdmit=[&](const Bus::Request &r){return na_.admit(r.ticket,r.frame,r.stop);};
   auto backupAdmit=[&](const Bus::Request &r){return ea_.tryAdmitReply(r,now);};
   sharedNrf_->step(now,age,nrfInitialized_&&nrf_.txPending(),enabled&&nrfInitialized_&&nrf_.ready(),
    [&](const RadioFrame &f){return boatReplyShape(f,boatId_,remoteId_,false);},primaryAdmit,primaryAdmit);
   sharedE22_->step(now,age,e22Initialized_&&e22_.txPending(),
    enabled&&e22Initialized_&&e22AirtimeConfigured_&&e22_.ready(),
    [&](const RadioFrame &f){return boatReplyShape(f,boatId_,remoteId_,true);},backupAdmit,backupAdmit);
   return;
  }
  ns_.step(now,age,enabled&&nrfInitialized_,[&](const Bus::Request &r){return na_.admit(r.ticket,r.frame,r.stop);});
  es_.step(now,age,enabled&&e22Initialized_&&e22AirtimeConfigured_,
   [&](const Bus::Request &r){return ea_.admit(r.ticket,r.frame,now,r.stop);});
 }
 // Software closure only. Caller invalidates tickets before shutdown.
 // No claim of RF quiescence or completed application ACK.
 void shutdown(){
  if(closed_)return;
  closed_=true;
  if(nrfInitialized_)nrf_.shutdownInitialized();
  if(e22Initialized_)e22_.shutdownLocal();
 }
};
}

#include <atomic>
namespace flytec_r30h120 {
// Not instantiated by production. Owner must retain this object and buses
// until the task has exited. Never start alongside the legacy radio owner.
class PhysicalTask {
 using Bus=flytec_r30h118::Transport<RadioFrame>;
 using Events=flytec_r30h118::PhyEvents<RadioFrame>;
public:
 struct Config {
  int ce,csn; SPIClass *spi; uint8_t channel; int sck,miso,mosi;
  E22BoardPins pins; bool primary,backup;
  flytec_rtos::LoRaAirtimeProfile backupAirtimeProfile;
  uint32_t backupRfTailMs;
  bool backupAirtimeEvidenceSigned;
 };
private:
 Config cfg_;
 Bus &nb_,&eb_; Events &ne_,&ee_;
 HandshakeLane<RadioFrame>* nh_=nullptr;
 HandshakeLane<RadioFrame>* eh_=nullptr;
 SharedPriority *np_=nullptr,*ep_=nullptr;
 uint8_t boatId_=0,remoteId_=0;
 Lifecycle life_;
 std::atomic<uint32_t> maxUs_{0},stackRaw_{0},cycles_{0};
 std::atomic<bool> exited_{false};
 static void entry(void *arg){static_cast<PhysicalTask*>(arg)->run();vTaskDelete(nullptr);}
 void run(){
  {
   // Dormant local lanes support the original compile-only task constructor.
   HandshakeLane<RadioFrame> unusedN,unusedE;
   SharedPriority unusedNP,unusedEP;
   SharedPhyService<RadioFrame> ns(nb_,nh_?*nh_:unusedN,np_?*np_:unusedNP);
   SharedPhyService<RadioFrame> es(eb_,eh_?*eh_:unusedE,ep_?*ep_:unusedEP);
   PhysicalWorker w(cfg_.ce,cfg_.csn,cfg_.spi,cfg_.channel,cfg_.sck,cfg_.miso,cfg_.mosi,
                    nb_,eb_,ne_,ee_,cfg_.backupAirtimeProfile,cfg_.backupRfTailMs,
                    cfg_.backupAirtimeEvidenceSigned);
   const bool bound=!nh_||w.bindShared(ns,es,boatId_,remoteId_);
   const bool ok=bound&&life_.phase()==Phase::Starting&&w.begin(true,cfg_.primary,cfg_.pins,cfg_.backup);
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
 // Owner-only, before start. External lanes and masks must outlive task exit.
 bool bindShared(HandshakeLane<RadioFrame>& nh,HandshakeLane<RadioFrame>& eh,SharedPriority& np,SharedPriority& ep,uint8_t boat,uint8_t remote){
  if(life_.phase()!=Phase::Idle||nh_||&nh==&eh||&np==&ep||&nb_==&eb_||&ne_==&ee_||!boat||!remote||boat==remote)return false;
  nh_=&nh;eh_=&eh;np_=&np;ep_=&ep;boatId_=boat;remoteId_=remote;return true;
 }
 bool start(bool commissioningAuthorized,bool legacyOwnerAbsent){
  if(!commissioningAuthorized||!legacyOwnerAbsent||(!cfg_.primary&&!cfg_.backup)||!life_.start())return false;
  if(xTaskCreatePinnedToCore(entry,"boat_phy",6144,this,1,nullptr,0)!=pdPASS){life_.finish(false);exited_.store(true);return false;}
  return true;
 }
 void stop(){life_.stop();if(np_)np_->publish(0);if(ep_)ep_->publish(0);nb_.invalidate(0);eb_.invalidate(0);if(nh_)nh_->close();if(eh_)eh_->close();}
 Phase phase()const{return life_.phase();}
 bool exited()const{return exited_.load(std::memory_order_acquire);}
 uint32_t maxUs()const{return maxUs_.load();}
 uint32_t stackRaw()const{return stackRaw_.load();}
 uint32_t cycles()const{return cycles_.load();}
};
}
