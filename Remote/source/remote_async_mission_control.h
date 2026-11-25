#pragma once
#include "mission_control_wire.h"
#include <stdint.h>
namespace flytec_rtos {
// Owner-only, experimental scheduler. Caller verifies HMAC, replay and
// primary capture and supplies independent mission authority, not ordinary TX.
struct MissionControlAuthority {
 bool session=false,primaryFresh=false,missionAuthority=false,stopReleased=false;
 bool neutralLocal=false,neutralLease=false,stationary=false,actuatorReady=false;
 bool batteryOk=false,failsafe=false;
};
class AsyncMissionControl {
public:
 enum class State:uint8_t{Unbound,Idle,Pending,Accepted,Rejected,Unknown};
 static constexpr uint32_t Deadline=1800U,RetryGap=400U,TxLifetime=100U,StatusAge=1000U;
 State state()const{return state_;}
 bool inFlight()const{return ticket_!=0;}
 uint16_t transaction()const{return transaction_;}
 MissionControlPhase phase()const{return phase_;}
 uint8_t reservations()const{return attempts_;}
 bool fresh(uint32_t now)const{return seen_&&uint32_t(now-at_)<StatusAge;}
 // Only after explicit upload handoff. The caller supplies original upload
 // start, so delayed binding cannot extend the 9-second START budget.
 bool bind(uint16_t id,uint8_t epoch,uint32_t uploadStart,uint32_t captured,uint32_t now,bool handedOff){
  if(state_!=State::Unbound||!handedOff||!id||!epoch||uint32_t(now-captured)>=100U||
     uint32_t(captured-uploadStart)>=9000U||uint32_t(now-uploadStart)>=9000U)return false;
  transaction_=id;epoch_=epoch;uploadStart_=uploadStart;phase_=MC_PREPARED;seen_=true;at_=captured;state_=State::Idle;return true;
 }
 bool request(MissionControlOp op,uint32_t now,const MissionControlAuthority& a){
  if(!transaction_||state_==State::Pending||inFlight()||nextId_==UINT16_MAX||
     (state_==State::Unknown&&op!=MCTRL_CANCEL)||!allowed(op,now,a))return false;
  request_={op,transaction_,++nextId_};started_=now;offered_=false;attempts_=0;state_=State::Pending;return true;
 }
 // False requires owner to revoke outstanding generation before reuse.
 bool maintain(uint32_t now,const MissionControlAuthority& a){
  if(state_==State::Pending&&(!authorized(request_.op,now,a)||uint32_t(now-started_)>=Deadline||
      (attempts_>=3&&!inFlight()&&uint32_t(now-offeredAt_)>=RetryGap)))state_=State::Unknown;
  if(inFlight()&&uint32_t(now-offeredAt_)>=lifetime_)state_=State::Unknown;
  return state_!=State::Unknown;
 }
 bool due(uint32_t now,const MissionControlAuthority& a,MissionControlRequest& out){
  if(!maintain(now,a)||state_!=State::Pending||inFlight()||attempts_>=3||
     (offered_&&uint32_t(now-offeredAt_)<RetryGap))return false;
  out=request_;return true;
 }
 uint32_t lifetime(uint32_t now)const{
  const uint32_t elapsed=now-started_;if(elapsed>=Deadline)return 0;
  uint32_t remaining=Deadline-elapsed;
  if(request_.op==MCTRL_START){const uint32_t age=now-uploadStart_;if(age>=9000U)return 0;if(9000U-age<remaining)remaining=9000U-age;}
  return remaining<TxLifetime?remaining:TxLifetime;
 }
 bool queued(uint32_t ticket,uint32_t generation,uint16_t sequence,uint8_t epoch,uint32_t now,uint32_t ttl){
  if(state_!=State::Pending||inFlight()||!ticket||!generation||!sequence||epoch!=epoch_||!ttl||ttl!=lifetime(now)||
     attempts_>=3||(offered_&&uint32_t(now-offeredAt_)<RetryGap))return false;
  ticket_=ticket;generation_=generation;sequence_=sequence;offeredAt_=now;lifetime_=ttl;offered_=true;++attempts_;return true;
 }
 bool complete(uint32_t ticket,uint32_t generation,uint16_t sequence,uint8_t epoch,uint16_t transaction,uint32_t created,uint32_t ttl){
  if(!inFlight()||ticket!=ticket_||generation!=generation_||sequence!=sequence_||epoch!=epoch_||
     transaction!=transaction_||created!=offeredAt_||ttl!=lifetime_)return false;
  ticket_=0;return true; // local evidence only, never a control ACK
 }
 bool observe(const MissionControlFeedback& f,uint8_t epoch,uint32_t captured,uint32_t now,bool authenticatedPrimary){
  if(!authenticatedPrimary||!transaction_||epoch!=epoch_||!missionFeedbackValid(f)||f.transaction!=transaction_||
     uint32_t(now-captured)>=100U||(seen_&&uint32_t(captured-at_)>=0x80000000U))return false;
  if(f.result!=MC_STATUS){
   if(state_!=State::Pending||!offered_||f.request!=request_.request||f.op!=request_.op||
      uint32_t(captured-offeredAt_)>=Deadline||uint32_t(captured-started_)>=Deadline||!lifetime(now)||
      (attempts_>=3&&!inFlight()&&uint32_t(now-offeredAt_)>=RetryGap))return false;
   if(f.result==MC_ACCEPTED&&!acceptedPhase(request_.op,f.phase)){state_=State::Unknown;return false;}
   state_=f.result==MC_ACCEPTED?State::Accepted:State::Rejected;
  }
  phase_=f.phase;seen_=true;at_=captured;return true;
 }
 // After actual transport revocation only. Unknown remains locked, even if
 // status arrives; a new explicit CANCEL is the sole recovery operation.
 void transportRevoked(){ticket_=0;if(state_!=State::Unbound)state_=State::Unknown;}
 void resetAfterSession(){*this=AsyncMissionControl{};}
private:
 bool authorized(MissionControlOp op,uint32_t now,const MissionControlAuthority& a)const{
  if(!a.session||!a.primaryFresh||!a.missionAuthority)return false;
  if(op==MCTRL_CANCEL)return !a.failsafe&&a.batteryOk;
  if(!fresh(now)||!a.stopReleased||!a.actuatorReady||!a.batteryOk||a.failsafe)return false;
  if(op==MCTRL_START||op==MCTRL_RESUME){
   if(!a.neutralLocal||!a.neutralLease||!a.stationary)return false;
   if(op==MCTRL_START&&uint32_t(now-uploadStart_)>=9000U)return false;
  }
  return op==MCTRL_START||op==MCTRL_RESUME||op==MCTRL_PAUSE;
 }
 bool allowed(MissionControlOp op,uint32_t now,const MissionControlAuthority& a)const{
  if(!authorized(op,now,a))return false;
  switch(op){
   case MCTRL_START:return phase_==MC_PREPARED;
   case MCTRL_PAUSE:return phase_==MC_OUTBOUND||phase_==MC_RETURNING;
   case MCTRL_RESUME:return phase_==MC_PAUSED||phase_==MC_OVERRIDE_NEUTRAL||phase_==MC_OVERRIDE_ACTIVE;
   case MCTRL_CANCEL:return true;
   default:return false;
  }
 }
 static bool acceptedPhase(MissionControlOp op,MissionControlPhase p){
  if(op==MCTRL_CANCEL)return p==MC_ABORTED;
  if(op==MCTRL_PAUSE)return p==MC_PAUSED;
  return p==MC_OUTBOUND||p==MC_RETURNING||p==MC_RELEASING||p==MC_COMPLETE;
 }
 State state_=State::Unbound;MissionControlRequest request_{};MissionControlPhase phase_=MC_IDLE;
 uint32_t uploadStart_=0,started_=0,offeredAt_=0,lifetime_=0,ticket_=0,generation_=0,at_=0;
 uint16_t transaction_=0,nextId_=0,sequence_=0;uint8_t epoch_=0,attempts_=0;bool seen_=false,offered_=false;
};
}
