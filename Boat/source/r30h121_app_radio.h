#pragma once
#include "r30h118_transport.h"
#include "r30h118_phy_events.h"
namespace flytec_r30h121 {
using namespace flytec_r30h118;
enum class TxState:uint8_t { Idle, Queued, Admitted, Rejected, LocalComplete, Failed, Uncertain };
template<class Frame> class AppRadio {
 using Bus=Transport<Frame>;
 using Events=PhyEvents<Frame>;
 Bus &bus_;
 Events &events_;
 Ticket ticket_{};
 uint32_t submittedAt_=0;
 Frame stopFrame_{};
 uint32_t stopAt_=0;
 bool stopRetry_=false;
 typename Events::Health health_{};
 bool healthSeen_=false, admissionSeen_=false, admitted_=false, outcomeSeen_=false;
 PhyOutcome outcome_=PhyOutcome::Uncertain;
 TxState state_=TxState::Idle;
public:
 AppRadio(Bus &bus,Events &events):bus_(bus),events_(events){}
 bool reset(uint8_t epoch){
  stopRetry_=false;state_=TxState::Idle;admissionSeen_=admitted_=outcomeSeen_=false;
  healthSeen_=false;ticket_={};return bus_.invalidate(epoch);
 }
 bool healthy(uint32_t now,uint32_t age)const{
  return healthSeen_&&!events_.stickyFault()&&health_.configured&&health_.ready&&!health_.fault&&age&&uint32_t(now-health_.observedAt)<age;
 }
 bool submit(const Frame &frame,uint32_t now,bool stop){
  if(state_==TxState::Queued||state_==TxState::Admitted)return false;
  Ticket next{};
  if(!bus_.submit(frame,now,stop,&next))return false;
  ticket_=next;submittedAt_=now;admissionSeen_=admitted_=outcomeSeen_=false;state_=TxState::Queued;return true;
 }
 // Timeout revokes queued work; an already active PHY call cannot be revoked.
 bool expire(uint32_t now,uint32_t timeout){
  if((state_!=TxState::Queued&&state_!=TxState::Admitted)|| (timeout&&uint32_t(now-submittedAt_)<timeout))return false;
  bus_.invalidate(0);state_=TxState::Uncertain;return true;
 }
 bool preemptStop(const Frame &frame,uint32_t now,uint8_t epoch){
  bus_.latchStop();stopRetry_=false;if(!bus_.invalidate(epoch)){state_=TxState::Uncertain;return false;}
  state_=TxState::Uncertain;stopFrame_=frame;stopAt_=now;stopRetry_=true;
  return serviceStop(now,100);
 }
 // Owner retains a saturated STOP request with its original timestamp.
 bool serviceStop(uint32_t now,uint32_t maxAge){
  if(!stopRetry_)return false;
  if(!maxAge||uint32_t(now-stopAt_)>=maxAge){stopRetry_=false;bus_.invalidate(0);return false;}
  if(!submit(stopFrame_,stopAt_,true))return false;
  stopRetry_=false;return true;
 }
 bool stopRetryPending()const{return stopRetry_;}
 bool rearm(uint8_t epoch,bool authorized){
  if(!authorized||stopRetry_||state_==TxState::Queued||state_==TxState::Admitted)return false;
  if(!bus_.rearm(epoch))return false;
  state_=TxState::Idle;return true;
 }
 void poll(){
  if(events_.latestHealth(health_))healthSeen_=true;
  bool admitted=false;
  for(unsigned i=0;i<3;++i)if(bus_.poll(admitted)){admissionSeen_=true;admitted_=admitted;}
  typename Events::Outcome event{};
  for(unsigned i=0;i<4&&events_.takeOutcome(event);++i)
   if((state_==TxState::Queued||state_==TxState::Admitted)&&ticket_.generation==bus_.activeGeneration()&&sameTicket(ticket_,event.ticket)){outcomeSeen_=true;outcome_=event.result;}
  if(state_!=TxState::Queued&&state_!=TxState::Admitted)return;
  if(ticket_.generation!=bus_.activeGeneration()){state_=TxState::Uncertain;return;}
  if(admissionSeen_){
   if(!admitted_){state_=TxState::Uncertain;return;}
   state_=TxState::Admitted;
   if(outcomeSeen_)state_=outcome_==PhyOutcome::LocalComplete?TxState::LocalComplete:outcome_==PhyOutcome::Failed?TxState::Failed:TxState::Uncertain;
  }
 }
 bool receive(typename Events::Rx &rx){return events_.takeRx(rx);}
 TxState state()const{return state_;}
};
}
