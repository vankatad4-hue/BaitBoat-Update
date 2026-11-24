#pragma once
#include "r30h123_owner_link.h"
#include "boat_r30h19_status_outbox.h"
namespace flytec_r30h123 {
// Owner-only service. Before polling the owner, reconcile replacements,
// expiry and lost trust; after polling route the exact terminal result.
// STOP ACK has precedence. Status comes only from real local feedback.
template<class Frame> class StatusService {
 OwnerLink<Frame>& owner_;
 boat_r30h19::StatusOutbox& box_;
 Ticket ticket_{};
 uint64_t revision_=0;
 bool tracking_=false;
public:
 StatusService(OwnerLink<Frame>& owner,boat_r30h19::StatusOutbox& box):owner_(owner),box_(box){}
 bool pending()const{return tracking_;}
 void reconcile(uint32_t now,bool sessionReady,bool linkReady,bool stopAckPending){
  box_.due(now,sessionReady,linkReady,true,stopAckPending);
  if(tracking_&&(!box_.pending()||revision_!=box_.revision()||stopAckPending)){
   owner_.revokeStatus(ticket_);tracking_=false;
  }
 }
 void reset(){if(tracking_)owner_.revokeStatus(ticket_);tracking_=false;box_.reset();}
 template<class Build,class Verify> bool step(uint32_t now,bool sessionReady,bool linkReady,bool stopAckPending,Build build,Verify verify){
  reconcile(now,sessionReady,linkReady,stopAckPending);
  if(tracking_||!box_.due(now,sessionReady,linkReady,false,stopAckPending))return false;
  const auto age=box_.remainingAge(now);
  const auto snapshot=box_.status();
  Frame frame{};
  if(!age||!build(snapshot,frame))return false;
  const auto revision=box_.revision();
  if(!owner_.submitStatus(frame,now,age,snapshot.stopState==1U,verify))return false;
  ticket_=owner_.currentTicket();revision_=revision;tracking_=true;return true;
 }
 bool complete(const Ticket& t,TxState state){
  if(!tracking_||!sameTicket(t,ticket_)||state==TxState::Idle||state==TxState::Queued||state==TxState::Admitted)return false;
  tracking_=false;
  if(state==TxState::LocalComplete)box_.sentRevision(revision_);
  return true;
 }
};
}
