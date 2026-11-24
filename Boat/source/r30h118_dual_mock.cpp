#if defined(FLYTEC_R30H118_DUAL_MOCK)
#include <Arduino.h>
#include "r30h118_phy_adapters.h"
#include "r30h118_transport.h"
#include "r30h118_stop_retry.h"
using namespace flytec_r30h118;
namespace {
struct Nrf {
 bool pending=false;
 bool ready()const{return true;}
 bool txPending()const{return pending;}
 bool lastTxSuccess()const{return true;}
 bool sendFrame(const uint32_t&){pending=true;return true;}
 bool receiveFrame(uint32_t&){return false;}
 void update(){pending=false;}
};
struct E22 {
 enum class Admission {Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 using O=flytec_rtos::E22TxState::Outcome;
 O state=O::Idle;
 bool configured()const{return true;}
 bool transportFault()const{return state==O::Uncertain;}
 bool txPending()const{return state==O::Pending;}
 bool ready()const{return !txPending()&&!transportFault();}
 O txOutcome()const{return state;}
 Admission trySendFrame(const uint32_t&,uint32_t){state=O::Pending;return Admission::Accepted;}
 bool receiveFrame(uint32_t&){return false;}
 void update(){if(txPending())state=O::Uncertain;}
};
Transport<uint32_t> primary,backup;
StopRetry<uint32_t> primaryStop;
PhyEvents<uint32_t> pe,be;
Nrf n; E22 e;
NrfAdapter<uint32_t,Nrf> na(n,pe);
E22Adapter<uint32_t,E22> ea(e,be);
std::atomic<bool> stop{false},done{false};
bool launched=false,reported=false,gotP=false,gotB=false,okP=false,okB=false;
uint32_t began=0;
template<class Admit> void service(Transport<uint32_t>& bus,Transport<uint32_t>::Request& r,bool& held,bool& result,uint32_t now,Admit admit){
 if(held){if(!bus.complete(r,result))return;held=false;}
 if(stop.load(std::memory_order_acquire)||!bus.take(r))return;
 result=bus.allows(r,now,1000U)&&admit(r);
 held=!bus.complete(r,result);
}
void task(void*){
 Transport<uint32_t>::Request p{},b{};bool ph=false,bh=false,pr=false,br=false;
 while(!stop.load(std::memory_order_acquire)){
  uint32_t now=millis();na.poll(now);ea.poll(now);
  service(primary,p,ph,pr,now,[&](const auto& r){return na.admit(r.ticket,r.frame);});
  service(backup,b,bh,br,now,[&](const auto& r){return ea.admit(r.ticket,r.frame,now);});
  vTaskDelay(1);
 }
 done.store(true,std::memory_order_release);vTaskDelete(nullptr);
}
}
void setup(){
 Serial.begin(115200);began=millis();
 primary.invalidate(1);backup.invalidate(1);
 primary.submit(1,began,false);
 primaryStop.arm(primary,1,2,began);primaryStop.service(primary,began,1000U);
 primaryStop.arm(primary,1,4,began); // saturate reserved slot with obsolete STOP
 primaryStop.service(primary,began,1000U);
 backup.submit(3,began,false);
 launched=xTaskCreatePinnedToCore(task,"dualMock",4096,nullptr,1,nullptr,0)==pdPASS;
 if(!launched)Serial.println("DUAL MOCK START FAIL");
}
void loop(){
 bool admitted=false;primary.poll(admitted);backup.poll(admitted);
 if(launched&&!stop.load()&&primaryStop.pending())primaryStop.service(primary,millis(),1000U);
 PhyEvents<uint32_t>::Health h{};while(pe.takeHealth(h)){}while(be.takeHealth(h)){}
 PhyEvents<uint32_t>::Outcome o{};
 while(pe.takeOutcome(o)){if(o.ticket.generation!=primary.activeGeneration())continue;gotP=true;okP=o.result==PhyOutcome::LocalComplete&&o.ticket.generation==3;}
 while(be.takeOutcome(o)){if(o.ticket.generation!=backup.activeGeneration())continue;gotB=true;okB=o.result==PhyOutcome::Uncertain;}
 if(launched&&!stop.load()&&((gotP&&gotB)||uint32_t(millis()-began)>=5000U)){
  primaryStop.reset();primary.invalidate(0);backup.invalidate(0);stop.store(true,std::memory_order_release);
 }
 if(done.load(std::memory_order_acquire)&&!reported){reported=true;Serial.println(gotP&&gotB&&okP&&okB?"DUAL MOCK PASS":"DUAL MOCK FAIL");}
 delay(1);
}
#endif
