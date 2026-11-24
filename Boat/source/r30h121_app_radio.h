#pragma once
#include "r30h118_transport.h"
#include "r30h118_phy_events.h"
#include "r30h120_handshake_lane.h"
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
 bool terminalTaken_=false;
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
  ticket_=next;terminalTaken_=false;submittedAt_=now;admissionSeen_=admitted_=outcomeSeen_=false;state_=TxState::Queued;return true;
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
 // Bounded owner-side RX filtering. Capture age is not authentication.
 bool receiveFresh(typename Events::Rx &rx,uint32_t now,uint32_t maxAge){
  for(unsigned i=0;i<4&&events_.takeRx(rx);++i)
   if(maxAge&&uint32_t(now-rx.capturedAt)<maxAge)return true;
  return false;
 }
 // Owner consumes a correlated terminal result once; never an authenticated ACK.
 Ticket currentTicket()const{return ticket_;}
 bool takeTerminal(Ticket &ticket,TxState &result){
  if(terminalTaken_||!ticket_.serial||state_==TxState::Idle||state_==TxState::Queued||state_==TxState::Admitted)return false;
  ticket=ticket_;result=state_;terminalTaken_=true;return true;
 }
 bool receive(typename Events::Rx &rx){return events_.takeRx(rx);}
 TxState state()const{return state_;}
};

// Owner-side STOP ACK correlation. One instance per link; reset on session
// change or outbox replacement. LocalComplete is not Remote confirmation.
class StopAckCorrelation {
 Ticket expected_{};
 uint16_t command_=0;
 uint8_t epoch_=0;
 bool pending_=false;
public:
 void reset(){pending_=false;}
 bool track(const Ticket &t,uint16_t command,uint8_t epoch){
  if(pending_||!command||!epoch||!t.generation||!t.serial||t.epoch!=epoch||t.domain!=TicketDomain::Application)return false;
  expected_=t;command_=command;epoch_=epoch;pending_=true;return true;
 }
 template<class Outbox> bool complete(Outbox &box,const Ticket &t,TxState state){
  if(!pending_||!sameTicket(t,expected_))return false;
  if(state!=TxState::LocalComplete&&state!=TxState::Failed&&state!=TxState::Uncertain&&state!=TxState::Rejected)return false;
  pending_=false;
  const auto ack=box.ticket();
  if(!box.pending()||!box.inFlight()||ack.commandId!=command_||ack.epoch!=epoch_)return false;
  box.completed(state==TxState::LocalComplete);return true;
 }
};
// Owner-side coordinator: distinct queues and health for each physical link.
// External callbacks own frame construction, authentication and handshake routing.
template<class Frame> class DualAppRadio {
 AppRadio<Frame> primary_,backup_;
public:
 DualAppRadio(Transport<Frame>& p,PhyEvents<Frame>& pe,Transport<Frame>& b,PhyEvents<Frame>& be):primary_(p,pe),backup_(b,be){}
 AppRadio<Frame>& primary(){return primary_;}
 AppRadio<Frame>& backup(){return backup_;}
 template<class Service> static bool serviceLink(AppRadio<Frame>& ep,uint32_t now,uint32_t age,uint8_t mask,Service service){
  if(ep.stopRetryPending()){ep.serviceStop(now,age);return false;}
  if(ep.state()==TxState::Queued||ep.state()==TxState::Admitted)return false;
  return flytec_r30h120::servicePriority(mask,ep.healthy(now,age),[&](flytec_r30h120::TxPriority p){return service(ep,p);});
 }
 template<class P,class B> void step(uint32_t now,uint32_t healthAge,uint32_t txTimeout,uint8_t pm,uint8_t bm,P ps,B bs){
  primary_.poll();backup_.poll();
  primary_.expire(now,txTimeout);backup_.expire(now,txTimeout);
  serviceLink(primary_,now,healthAge,pm,ps);
  serviceLink(backup_,now,healthAge,bm,bs);
 }
};
}
