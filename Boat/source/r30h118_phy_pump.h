#pragma once
#include "r30h118_phy_events.h"
namespace flytec_r30h118 {
// Worker-local state. Driver callbacks must be nonblocking and worker-owned.
template<class Frame> class PhyPump {
 using Events=PhyEvents<Frame>;
 Events &events_;
 typename Events::Rx rx_{};
 typename Events::Outcome outcome_{};
 Ticket active_{};
 bool rxHeld_=false,txActive_=false,outcomeHeld_=false;
public:
 explicit PhyPump(Events &events):events_(events){}
 bool canAdmit()const{return !txActive_&&!outcomeHeld_;}
 bool track(const Ticket &t){
  if(!canAdmit()||!t.generation||!t.serial||!t.epoch)return false;
  active_=t;txActive_=true;return true;
 }
 bool terminal(PhyOutcome result,uint32_t now){
  if(!txActive_)return false;
  outcome_={active_,now,result};txActive_=false;outcomeHeld_=true;return true;
 }
 template<class Receive> void pollRx(uint32_t now,Receive receive){
  if(rxHeld_){if(!events_.publishRx(rx_))return;rxHeld_=false;}
  for(unsigned i=0;i<4;++i){
   if(!receive(rx_.frame))return;
   rx_.capturedAt=now;
   if(!events_.publishRx(rx_)){rxHeld_=true;return;}
  }
 }
 void flushOutcome(){if(outcomeHeld_&&events_.publishOutcome(outcome_))outcomeHeld_=false;}
 bool publishHealth(const typename Events::Health &h){return events_.publishHealth(h);}
};
template<class Health> bool healthyFresh(const Health &h,uint32_t now,uint32_t maxAge){
 return h.configured&&h.ready&&!h.fault&&maxAge&&uint32_t(now-h.observedAt)<maxAge;
}
}
