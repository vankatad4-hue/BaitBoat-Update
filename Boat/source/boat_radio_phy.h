#pragma once
// Consolidated Boat radio component; original types and owner contracts retained.
#include "boat_radio_transport.h"
namespace flytec_r30h118 {
enum class PhyOutcome:uint8_t { LocalComplete, Failed, Uncertain };
template<class Frame> class PhyEvents {
public:
 struct Rx { Frame frame; uint32_t capturedAt; };
 struct Health { uint32_t observedAt; bool configured,ready,fault,pending; };
 struct Outcome { Ticket ticket; uint32_t observedAt; PhyOutcome result; bool stop=false; };
private:
 SpscQueue<Rx,5> rx_;
 SpscQueue<Health,5> health_;
 std::atomic<bool> stickyFault_{false};
 SpscQueue<Outcome,5> outcomes_;
 SpscQueue<Outcome,5> stopOutcomes_;
 SpscQueue<Outcome,5> handshakeOutcomes_;
public:
 // Worker publishes; application consumes. Separate instance per physical link.
 // False means full: caller retains critical events and retries; no overwrite.
 bool publishRx(const Rx &v){return rx_.push(v);}
 bool publishHealth(const Health &v){
  if(v.fault)stickyFault_.store(true,std::memory_order_release);
  return health_.push(v);
 }
 // Fault survives a full queue. Reset only by reconstructing after worker join.
 bool stickyFault()const{return stickyFault_.load(std::memory_order_acquire);}
 bool publishOutcome(const Outcome &v){
  switch(v.ticket.domain){
   case TicketDomain::Application:return v.stop?stopOutcomes_.push(v):outcomes_.push(v);
   case TicketDomain::Handshake:return handshakeOutcomes_.push(v);
   default:return false;
  }
 }
 bool takeRx(Rx &v){return rx_.pop(v);}
 bool takeHealth(Health &v){
  if(!health_.pop(v))return false;
  if(stickyFault()){v.fault=true;v.ready=false;}
  return true;
 }
 bool latestHealth(Health &v){
  bool found=false;Health next{};
  for(unsigned i=0;i<4&&takeHealth(next);++i){v=next;found=true;}
  if(stickyFault()){v.fault=true;v.ready=false;}
  return found;
 }
 bool takeOutcome(Outcome &v){return stopOutcomes_.pop(v)||outcomes_.pop(v);}
 bool takeHandshakeOutcome(Outcome &v){return handshakeOutcomes_.pop(v);}
};
inline bool sameTicket(const Ticket &a,const Ticket &b){
 return a.domain==b.domain&&a.generation==b.generation&&a.serial==b.serial&&a.epoch==b.epoch;
}
}

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
 void flushOutcome(){const unsigned order[3]={1,0,2};for(unsigned j=0;j<3;++j){const unsigned i=order[j];if(held_[i]&&events_.publishOutcome(outcomes_[i]))held_[i]=false;}}
 bool publishHealth(const typename Events::Health &h){return events_.publishHealth(h);}
};
template<class Health> bool healthyFresh(const Health &h,uint32_t now,uint32_t maxAge){
 return h.configured&&h.ready&&!h.fault&&maxAge&&uint32_t(now-h.observedAt)<maxAge;
}
}

#include "e22_tx_state.h"
namespace flytec_r30h118 {
template<class Frame,class Driver> class NrfAdapter {
 Driver &driver_; PhyPump<Frame> pump_; bool active_=false;
public:
 NrfAdapter(Driver &d,PhyEvents<Frame>& e):driver_(d),pump_(e){}
 bool admit(const Ticket &t,const Frame &f,bool stop=false){
  if(!pump_.canAdmit(t,stop)||!driver_.ready()||driver_.txPending()||!t.generation||!t.serial||!t.epoch)return false;
  if(!driver_.sendFrame(f))return false;
  active_=pump_.track(t,stop);return active_;
 }
 void poll(uint32_t now){
  driver_.update();
  if(active_&&!driver_.txPending()){
   pump_.terminal(!driver_.ready()?PhyOutcome::Uncertain:driver_.lastTxSuccess()?PhyOutcome::LocalComplete:PhyOutcome::Failed,now);active_=false;
  }
  pump_.flushOutcome();
  pump_.pollRx(now,[&](Frame &f){return driver_.receiveFrame(f);});
  pump_.publishHealth({now,driver_.ready(),driver_.ready(),!driver_.ready(),driver_.txPending()});
 }
};
template<class Frame,class Driver> class E22Adapter {
 Driver &driver_; PhyPump<Frame> pump_; bool active_=false;
public:
 E22Adapter(Driver &d,PhyEvents<Frame>& e):driver_(d),pump_(e){}
 bool admit(const Ticket &t,const Frame &f,uint32_t now,bool stop=false){
  if(!pump_.canAdmit(t,stop)||!driver_.ready()||!t.generation||!t.serial||!t.epoch)return false;
  using A=typename Driver::Admission;
  const auto result=driver_.trySendFrame(f,now);
  if(result==A::Uncertain){pump_.track(t,stop);pump_.terminal(PhyOutcome::Uncertain,now);return false;}
  if(result!=A::Accepted)return false;
  active_=pump_.track(t,stop);return active_;
 }
 void poll(uint32_t now){
  driver_.update();
  if(active_&&!driver_.txPending()){
   using O=flytec_rtos::E22TxState::Outcome;
   pump_.terminal(driver_.txOutcome()==O::LocalReady&&!driver_.transportFault()?PhyOutcome::LocalComplete:PhyOutcome::Uncertain,now);active_=false;
  }
  pump_.flushOutcome();
  pump_.pollRx(now,[&](Frame &f){return driver_.receiveFrame(f);});
  pump_.publishHealth({now,driver_.configured(),driver_.ready(),driver_.transportFault(),driver_.txPending()});
 }
};
}

namespace flytec_r30h118 {
// One per link. Register exact submitted ticket; invalidate on STOP/session change.
class OutcomeGate {
 Ticket expected_{};
 bool pending_=false;
public:
 void reset(){pending_=false;}
 bool expect(const Ticket &t){
  if(pending_||!t.generation||!t.serial||!t.epoch)return false;
  expected_=t;pending_=true;return true;
 }
 bool accept(const Ticket &t,uint32_t activeGeneration){
  if(!pending_||!activeGeneration||t.generation!=activeGeneration||!sameTicket(t,expected_))return false;
  pending_=false;return true;
 }
};
}
