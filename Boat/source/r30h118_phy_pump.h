#pragma once
#include "r30h118_phy_events.h"
namespace flytec_r30h118 {
// Worker-local state. Driver callbacks must be nonblocking and worker-owned.
template<class Frame> class PhyPump {
 using Events=PhyEvents<Frame>;
 Events &events_;
 typename Events::Rx rx_{};
 typename Events::Outcome outcomes_[3]{};
 bool held_[3]{};
 unsigned activeLane_=0;
 static unsigned lane(const Ticket &t,bool stop){return t.domain==TicketDomain::Handshake?2:stop?1:0;}
 Ticket active_{};
 bool rxHeld_=false,txActive_=false;
public:
 explicit PhyPump(Events &events):events_(events){}
 bool canAdmit()const{return !txActive_&&!held_[0]&&!held_[1]&&!held_[2];}
 bool canAdmit(const Ticket &t,bool stop=false)const{return !txActive_&&!held_[lane(t,stop)];}
 bool track(const Ticket &t,bool stop=false){
  if(!canAdmit(t,stop)||!t.generation||!t.serial||!t.epoch)return false;
  active_=t;activeLane_=lane(t,stop);txActive_=true;return true;
 }
 bool terminal(PhyOutcome result,uint32_t now){
  if(!txActive_)return false;
  outcomes_[activeLane_]={active_,now,result,activeLane_==1};txActive_=false;held_[activeLane_]=true;return true;
 }
 template<class Receive> void pollRx(uint32_t now,Receive receive){
  if(rxHeld_){if(!events_.publishRx(rx_))return;rxHeld_=false;}
  for(unsigned i=0;i<4;++i){
   if(!receive(rx_.frame))return;
   rx_.capturedAt=now;
   if(!events_.publishRx(rx_)){rxHeld_=true;return;}
  }
 }
 void flushOutcome(){for(unsigned j=0;j<3;++j)if(const unsigned i=(j==0?1:j==1?0:2);true)if(held_[i]&&events_.publishOutcome(outcomes_[i]))held_[i]=false;}
 bool publishHealth(const typename Events::Health &h){return events_.publishHealth(h);}
};
template<class Health> bool healthyFresh(const Health &h,uint32_t now,uint32_t maxAge){
 return h.configured&&h.ready&&!h.fault&&maxAge&&uint32_t(now-h.observedAt)<maxAge;
}
}
