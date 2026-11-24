#if defined(FLYTEC_R30H118_DUAL_MOCK)
#include <Arduino.h>
#include "r30h118_phy_adapters.h"
#include "r30h118_transport.h"
#include "r30h120_phy_service.h"
#include "r30h118_stop_retry.h"
#include "r30h118_outcome_gate.h"
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
OutcomeGate pg,bg;
PhyEvents<uint32_t> pe,be;
Nrf n; E22 e;
NrfAdapter<uint32_t,Nrf> na(n,pe);
E22Adapter<uint32_t,E22> ea(e,be);
std::atomic<bool> stop{false},done{false};
bool launched=false,reported=false,gotP=false,gotB=false,okP=false,okB=false;
bool healthOK=false;
uint32_t began=0;
void task(void*){
 flytec_r30h120::PhyService<uint32_t> p(primary),b(backup);
 while(!stop.load(std::memory_order_acquire)){
  uint32_t now=millis();na.poll(now);ea.poll(now);
  p.step(now,1000U,!stop.load(std::memory_order_acquire),[&](const auto& r){return na.admit(r.ticket,r.frame);});
  b.step(now,1000U,!stop.load(std::memory_order_acquire),[&](const auto& r){return ea.admit(r.ticket,r.frame,now);});
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
 Ticket bt{};if(backup.submit(3,began,false,&bt))bg.expect(bt);
 launched=xTaskCreatePinnedToCore(task,"dualMock",4096,nullptr,1,nullptr,0)==pdPASS;
 if(!launched)Serial.println("DUAL MOCK START FAIL");
}
void loop(){
 bool admitted=false;primary.poll(admitted);backup.poll(admitted);
 if(launched&&!stop.load()&&primaryStop.pending()){Ticket pt{};if(primaryStop.service(primary,millis(),1000U,&pt))pg.expect(pt);}
 PhyEvents<uint32_t>::Health hp{},hb{};
 const bool haveP=pe.latestHealth(hp);be.latestHealth(hb);
 if(haveP)healthOK=healthyFresh(hp,millis(),1000U)&&!pe.stickyFault()&&be.stickyFault();
 if(pe.stickyFault())healthOK=false;
 PhyEvents<uint32_t>::Outcome o{};
 while(pe.takeOutcome(o)){if(!pg.accept(o.ticket,primary.activeGeneration()))continue;gotP=true;okP=o.result==PhyOutcome::LocalComplete&&o.ticket.generation==3;}
 while(be.takeOutcome(o)){if(!bg.accept(o.ticket,backup.activeGeneration()))continue;gotB=true;okB=o.result==PhyOutcome::Uncertain;}
 if(launched&&!stop.load()&&((gotP&&gotB&&healthOK)||uint32_t(millis()-began)>=5000U)){
  primaryStop.reset();primary.invalidate(0);backup.invalidate(0);stop.store(true,std::memory_order_release);
 }
 if(done.load(std::memory_order_acquire)&&!reported){reported=true;Serial.println(gotP&&gotB&&okP&&okB&&healthOK?"DUAL MOCK PASS":"DUAL MOCK FAIL");}
 delay(1);
}
#endif
