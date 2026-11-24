#pragma once
#include "r30h118_phy_pump.h"
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
