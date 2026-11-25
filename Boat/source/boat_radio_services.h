#pragma once
// Consolidated Boat radio component; original types and owner contracts retained.
#include "boat_radio_owner.h"
#include "boat_stop_delivery.h"
namespace flytec_r30h123 {
// One owner-only STOP ACK service per PHY. Caller must route terminal results
// after OwnerLink::poll and syncDeferred, before attempting another ACK.
// Reset together with the owner session. No driver calls or actuator authority.
template<class Frame,class Outbox> class StopAckService {
 OwnerLink<Frame>& owner_;
 Outbox& box_;
 const AckLink link_;
 StopAckCorrelation correlation_;
 boat_r30h18::StopAck snapshot_{};
 Ticket before_{},expected_{};
 bool deferred_=false,tracking_=false;
 bool sameSlot()const{
  const auto a=box_.ticket();
  return box_.pending()&&box_.inFlight()&&a.commandId==snapshot_.commandId&&a.epoch==snapshot_.epoch;
 }
 bool track(const Ticket& t){
  if(!correlation_.trackSnapshot(t,snapshot_))return false;
  expected_=t;tracking_=true;return true;
 }
public:
 StopAckService(OwnerLink<Frame>& owner,Outbox& box,AckLink link):owner_(owner),box_(box),link_(link),correlation_(link){}
 void reconcile(uint32_t now,bool sessionReady,bool linkReady){
  box_.due(now,sessionReady,linkReady,true);
  if(!pending()||sameSlot())return;
  if(deferred_)owner_.cancelDeferredStop();
  if(tracking_)owner_.revokeStatus(expected_);
  deferred_=tracking_=false;correlation_.reset();
 }
 void reset(){
  if(deferred_)owner_.cancelDeferredStop();
  if(tracking_)owner_.revokeStatus(expected_);
  deferred_=tracking_=false;correlation_.reset();box_.reset();
 }
 bool pending()const{return deferred_||tracking_;}
 // False does not mean an ACK was lost: the outbox may still retain it.
 template<class Build,class Verify> bool step(uint32_t now,bool sessionReady,bool linkReady,Build build,Verify verify){
  const bool due=box_.due(now,sessionReady,linkReady,pending());
  if(!due||pending()||owner_.stopRetryPending())return false;
  const auto age=box_.remainingAge(now);
  if(!age)return false;
  Frame frame{};
  snapshot_=box_.ticket();
  if(!build(snapshot_,frame))return false;
  before_=owner_.currentTicket();
  const bool queued=owner_.submitStop(frame,now,age,verify);
  if(!queued&&!owner_.stopRetryPending())return false;
  // Count the reservation as an attempt even if queue admission is deferred.
  box_.admitted(now);
  if(!queued){deferred_=true;return true;}
  if(track(owner_.currentTicket()))return true;
  if(sameSlot())box_.completed(false);
  return false;
 }
 void syncDeferred(){
  if(!deferred_||owner_.stopRetryPending())return;
  deferred_=false;
  const auto t=owner_.currentTicket();
  if(!sameTicket(t,before_)&&track(t))return;
  if(sameSlot())box_.completed(false);
 }
 // Return true for a consumed correlated ticket, even if its outbox was
 // replaced. Other terminal results belong to their own application consumer.
 bool complete(const Ticket& t,TxState state){
  if(!tracking_||!sameTicket(t,expected_))return false;
  if(state==TxState::Idle||state==TxState::Queued||state==TxState::Admitted)return false;
  tracking_=false;
  correlation_.complete(box_,link_,t,state);
  return true;
 }
};
}

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

namespace flytec_r30h123 {
// Core 1 owner only; one instance per link. The caller supplies trusted
// session/PHY readiness (not instantaneous TX-idle) and frame builders.
// No hardware task start, GPIO access, or automatic actuator rearm.
template<class Frame,class AckBox> class OwnerCycle {
 OwnerLink<Frame>& owner_;
 StopAckService<Frame,AckBox>& ack_;
 StatusService<Frame>& status_;
 AckBox& ackBox_;
 boat_r30h19::StatusOutbox& statusBox_;
public:
 OwnerCycle(OwnerLink<Frame>& owner,StopAckService<Frame,AckBox>& ack,
            StatusService<Frame>& status,AckBox& ackBox,boat_r30h19::StatusOutbox& statusBox):
  owner_(owner),ack_(ack),status_(status),ackBox_(ackBox),statusBox_(statusBox){}
 // Explicit session teardown revokes application work and resets feedback.
 // Full close is separate so root-key CONFIRM can survive application rollover.
 bool resetSession(uint8_t epoch){
  status_.reset();ack_.reset();return owner_.setSession(epoch);
 }
 void close(){status_.reset();ack_.reset();owner_.close();}
 template<class BuildAck,class BuildStatus,class Verify,class AppResult,class HsResult,class Ordinary>
 void step(uint32_t now,uint32_t appTimeout,uint32_t hsTimeout,bool sessionReady,bool linkReady,
           BuildAck buildAck,BuildStatus buildStatus,Verify verify,
           AppResult appResult,HsResult hsResult,Ordinary ordinary){
  // Revoke queued feedback before processing/admitting more work.
  ack_.reconcile(now,sessionReady,linkReady);
  status_.reconcile(now,sessionReady,linkReady,ackBox_.pending()||ack_.pending());
  owner_.poll(now,appTimeout,hsTimeout);
  ack_.syncDeferred();
  auto route=[&](const Ticket& t,TxState state){
   if(!ack_.complete(t,state)&&!status_.complete(t,state))appResult(t,state);
  };
  Ticket t{};TxState state=TxState::Idle;
  if(owner_.takeSuperseded(t,state))route(t,state);
  if(owner_.takeApplication(t,state))route(t,state);
  HandshakeTx hs=HandshakeTx::Idle;
  if(owner_.takeHandshake(t,hs))hsResult(t,hs);
  ack_.step(now,sessionReady,linkReady,buildAck,verify);
  if(ackBox_.pending()||ack_.pending())return;
  status_.step(now,sessionReady,linkReady,false,buildStatus,verify);
  if(statusBox_.pending()||status_.pending())return;
  if(sessionReady&&linkReady)ordinary(owner_);
 }
};
}
