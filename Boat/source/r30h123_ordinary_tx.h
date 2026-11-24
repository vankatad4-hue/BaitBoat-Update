#pragma once
#include "r30h123_owner_link.h"
namespace flytec_r30h123 {
// Owner-side correlation of ordinary transmissions, separate from STOP and
// handshake services. One instance per link; queue acceptance is not success.
template<class Frame> class OrdinaryTx {
 Ticket ticket_{};
 Frame frame_{};
 uint32_t queuedAt_=0,age_=0;
 bool pending_=false;
public:
 bool pending()const{return pending_;}
 void reset(){pending_=false;}
 // Owner revokes an exact pending snapshot when transport trust is lost.
 // A PHY call already in progress is not interruptible by this operation.
 bool cancel(OwnerLink<Frame>& owner){
  if(!pending_)return false;
  owner.revokeStatus(ticket_);pending_=false;return true;
 }
 template<class Verify> bool submit(OwnerLink<Frame>& owner,const Frame& frame,uint32_t now,TxPriority priority,uint32_t age,Verify verify,bool outboundFeedback=false){
  if(pending_||!age)return false;
  const bool queued=outboundFeedback?owner.submitFeedback(frame,now,priority,age,verify):owner.submitApplication(frame,now,priority,age,verify);
  if(!queued)return false;
  ticket_=owner.currentTicket();frame_=frame;queuedAt_=now;age_=age;pending_=true;return true;
 }
 template<class Result> bool complete(const Ticket& ticket,TxState state,uint32_t now,uint8_t epoch,Result result){
  if(!pending_||!sameTicket(ticket,ticket_)||state==TxState::Idle||state==TxState::Queued||state==TxState::Admitted)return false;
  pending_=false;
  // Never refresh a heartbeat or clear an outbox from an obsolete epoch,
  // or from a completion that is already outside its original age budget.
  if(!epoch||epoch!=frame_.sessionEpoch||uint32_t(now-queuedAt_)>=age_)state=TxState::Uncertain;
  result(frame_,state,queuedAt_);return true;
 }
};
}
