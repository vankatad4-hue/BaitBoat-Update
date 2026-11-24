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
 typename Events::Health health_{};
 bool healthSeen_=false, admissionSeen_=false, admitted_=false, outcomeSeen_=false;
 PhyOutcome outcome_=PhyOutcome::Uncertain;
 TxState state_=TxState::Idle;
public:
 AppRadio(Bus &bus,Events &events):bus_(bus),events_(events){}
 bool reset(uint8_t epoch){
  state_=TxState::Idle;admissionSeen_=admitted_=outcomeSeen_=false;
  healthSeen_=false;ticket_={};return bus_.invalidate(epoch);
 }
 bool healthy(uint32_t now,uint32_t age)const{
  return healthSeen_&&!events_.stickyFault()&&health_.configured&&health_.ready&&!health_.fault&&age&&uint32_t(now-health_.observedAt)<age;
 }
 bool submit(const Frame &frame,uint32_t now,bool stop){
  if(state_==TxState::Queued||state_==TxState::Admitted)return false;
  Ticket next{};
  if(!bus_.submit(frame,now,stop,&next))return false;
  ticket_=next;admissionSeen_=admitted_=outcomeSeen_=false;state_=TxState::Queued;return true;
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
   if(!admitted_){state_=outcomeSeen_?TxState::Uncertain:TxState::Rejected;return;}
   state_=TxState::Admitted;
   if(outcomeSeen_)state_=outcome_==PhyOutcome::LocalComplete?TxState::LocalComplete:outcome_==PhyOutcome::Failed?TxState::Failed:TxState::Uncertain;
  }
 }
 bool receive(typename Events::Rx &rx){return events_.takeRx(rx);}
 TxState state()const{return state_;}
};
}
