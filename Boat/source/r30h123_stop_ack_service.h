#pragma once
#include "r30h123_owner_link.h"
#include "boat_r30h18_stop_ack_outbox.h"
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
 void reset(){deferred_=tracking_=false;correlation_.reset();box_.reset();}
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
