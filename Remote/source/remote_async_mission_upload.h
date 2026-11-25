#pragma once
#include "mission_protocol.h"
#include <stdint.h>
// Core 1 only, isolated preparation scheduler. No radio or movement authority.
// The caller checks fresh primary auth, stopped/stationary state, battery and
// STOP priority. The caller must revoke outstanding transport on failure.
namespace flytec_rtos {
class AsyncMissionUpload {
public:
 enum class State:uint8_t{Idle,Active,Prepared,Failed};
 static constexpr uint32_t Deadline=9000U,RetryGap=500U,TxLifetime=100U;
 State state()const{return state_;}
 bool inFlight()const{return ticket_!=0;}
 uint8_t reservations()const{return attempts_;}
 uint16_t transaction()const{return plan_.transaction;}
 MissionOpcode opcode()const{return op_;}
 uint32_t started()const{return started_;}
 bool begin(const MissionPlan& plan,uint8_t epoch,uint32_t now,bool safe){
  if(!safe||!epoch||!missionPlan_valid(plan)||state_==State::Active||state_==State::Prepared||inFlight())return false;
  if(lastId_){const uint16_t delta=uint16_t(plan.transaction-lastId_);if(!delta||delta>=0x8000U)return false;}
  plan_=plan;lastId_=plan.transaction;epoch_=epoch;started_=now;op_=MISSION_BEGIN;attempts_=0;
  offered_=false;state_=State::Active;return true;
 }
 bool maintain(uint32_t now,bool safe){
  if(state_==State::Active||state_==State::Prepared){
   if(!safe||uint32_t(now-started_)>=Deadline){state_=State::Failed;return false;}
  }
  if(inFlight()&&uint32_t(now-offeredAt_)>=TxLifetime){state_=State::Failed;return false;}
  return state_!=State::Failed;
 }
 // Pure inspection apart from expiration. Busy/rejected submissions consume
 // neither reservation count nor retry spacing. One immutable payload per op.
 bool due(uint32_t now,bool safe,uint8_t out[8]){
  if(!maintain(now,safe)||state_!=State::Active||inFlight()||!out)return false;
  if(offered_&&uint32_t(now-offeredAt_)<RetryGap)return false;
  if(attempts_>=3){state_=State::Failed;return false;}
  return missionPlan_encode(plan_,op_,out);
 }
 bool queued(uint32_t ticket,uint32_t generation,uint16_t sequence,uint8_t epoch,uint32_t now){
  if(state_!=State::Active||inFlight()||!ticket||!generation||!sequence||epoch!=epoch_||
     uint32_t(now-started_)>=Deadline||attempts_>=3||
     (offered_&&uint32_t(now-offeredAt_)<RetryGap))return false;
  ticket_=ticket;generation_=generation;sequence_=sequence;offeredAt_=now;
  offered_=true;++attempts_;return true;
 }
 // Transport evidence retires the slot only. It is never a preparation ACK.
 // A peer ACK may arrive first; the next opcode still waits for slot release.
 bool complete(uint32_t ticket,uint32_t generation,uint16_t sequence,uint8_t epoch,uint16_t id,uint32_t created,uint32_t lifetime){
  if(!inFlight()||ticket!=ticket_||generation!=generation_||sequence!=sequence_||epoch!=epoch_||
     id!=plan_.transaction||created!=offeredAt_||lifetime!=TxLifetime)return false;
  ticket_=0;return true;
 }
 bool acknowledge(uint16_t id,MissionOpcode op,MissionReceive result,uint8_t epoch,
                  uint32_t captured,uint32_t now,bool authenticatedPrimary){
  if(state_!=State::Active||!offered_||!authenticatedPrimary||id!=plan_.transaction||op!=op_||epoch!=epoch_||
     uint32_t(now-captured)>=100U||uint32_t(captured-offeredAt_)>=Deadline||
     uint32_t(captured-started_)>=Deadline||uint32_t(now-started_)>=Deadline)return false;
  if(result==MISSION_REJECTED||result==MISSION_CANCELLED){state_=State::Failed;return true;}
  if(op==MISSION_COMMIT){
   if(result!=MISSION_PREPARED&&result!=MISSION_DUPLICATE)return false;
   state_=State::Prepared;offered_=false;return true;
  }
  if(result!=MISSION_PARTIAL&&result!=MISSION_DUPLICATE)return false;
  op_=static_cast<MissionOpcode>(uint8_t(op_)+1U);offered_=false;attempts_=0;return true;
 }
 bool prepared(uint32_t now)const{return state_==State::Prepared&&!inFlight()&&uint32_t(now-started_)<Deadline;}
 void fail(){state_=State::Failed;} // pending ticket remains until revoked or drained
 void transportRevoked(){ticket_=0;state_=State::Failed;offered_=false;}
 // Only after successful session generation revocation, never on a busy slot.
 void resetAfterSession(){*this=AsyncMissionUpload{};}
private:
 MissionPlan plan_{};MissionOpcode op_=MISSION_BEGIN;State state_=State::Idle;
 uint32_t started_=0,offeredAt_=0,ticket_=0,generation_=0;
 uint16_t sequence_=0,lastId_=0;uint8_t epoch_=0,attempts_=0;bool offered_=false;
};
}
