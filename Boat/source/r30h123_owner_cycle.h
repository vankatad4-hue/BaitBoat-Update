#pragma once
#include "r30h123_stop_ack_service.h"
#include "r30h123_status_service.h"
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
