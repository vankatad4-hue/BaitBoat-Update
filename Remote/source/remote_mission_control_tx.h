#pragma once
#include "remote_async_mission_control.h"
#include "mission_stick.h"
#include "mission_neutral_wire.h"
#include "remote_neutral_evidence.h"
#include "radio_protocol.h"
namespace flytec_rtos {
inline bool scopedMissionFrame(const RadioFrame& f){
 if(f.sourceId!=1||f.destinationId!=2||f.flags||!f.sequence||!f.sessionEpoch||!f.commandId)return false;
 if(f.type==RADIO_MSG_MISSION_CONTROL){
  MissionControlRequest r{};
  return missionControlDecode(f.payload,r)&&r.transaction==f.commandId&&r.op<=MCTRL_CANCEL;
 }
 if(f.type==RADIO_MSG_MISSION_STICKS){
  MissionStickInput s{};
  return missionStickDecode(f.payload,s)&&s.transaction==f.commandId&&s.neutral&&!s.left&&!s.right;
 }
 return false;
}
// Experimental owner-side signed control and neutral-only producer.
// No inference that local delivery proves Boat neutral lease. The caller
// supplies independent lease evidence and must revoke on false.
template<class Tx> class RemoteMissionControlTx {
public:
 explicit RemoteMissionControlTx(Tx& tx):tx_(tx){}
 AsyncMissionControl& control(){return control_;}
 const AsyncMissionControl& control()const{return control_;}
 bool neutralFresh(uint8_t epoch,uint32_t now,bool localNeutral)const{
  return neutralEvidence_.fresh(control_.transaction(),epoch,tx_.generation(),now,localNeutral);
 }
 bool observeNeutralReceipt(const RadioFrame& f,uint32_t captured,uint32_t now,bool authenticatedPrimary){
  uint16_t id=0,sequence=0;
  if(f.type!=RADIO_MSG_MISSION_NEUTRAL_ACK||f.sourceId!=2||f.destinationId!=1||f.flags||
     !missionNeutralDecode(f.payload,id,sequence)||id!=f.commandId||id!=control_.transaction())return false;
  return neutralEvidence_.receipt(id,sequence,f.sessionEpoch,tx_.generation(),captured,now,authenticatedPrimary);
 }
 void invalidateNeutral(){neutralEvidence_.reset();}
 bool observeControlFeedback(const MissionControlFeedback& f,uint8_t epoch,uint32_t captured,uint32_t now,bool authenticatedPrimary){
  if(!control_.observe(f,epoch,captured,now,authenticatedPrimary))return false;
  // Every correlated control response ends the previous neutral evidence
  // window. Especially PAUSE: the peer may have reset its local stick lease.
  if(f.result!=MC_STATUS)neutralEvidence_.reset();
  return true;
 }
 bool service(uint32_t now,const MissionControlAuthority& a,bool gap,uint8_t epoch,uint16_t& seq,uint16_t rekeyAt,const uint8_t key[32]){
  using L=typename Tx::Link;using Q=typename Tx::Submit;using S=AsyncMissionControl::State;
  if(!control_.maintain(now,a))return false;
  if(neutralTicket_&&(!neutralAllowed(a)||uint32_t(now-neutralAt_)>=NeutralLifetime))return false;
  if(!gap||tx_.pending(L::Primary))return true;
  MissionControlRequest r{};
  if(!control_.due(now,a,r))return control_.state()!=S::Unknown;
  if(!epoch||!key||rekeyAt<2||seq>=uint16_t(rekeyAt-1)||tx_.linkFault(L::Primary))return false;
  const uint32_t ttl=control_.lifetime(now);
  RadioFrame f{};radio_frameInit(f,RADIO_MSG_MISSION_CONTROL,1,2,uint16_t(seq+1),r.transaction,now,epoch);
  if(!ttl||!missionControlEncode(r,f.payload)||!scopedMissionFrame(f)||!radio_frameSeal(f,key))return false;
  ++seq;uint32_t ticket=0;const auto result=tx_.submitMission(f,now,ttl,now,ticket,a.missionAuthority);
  if(result==Q::Busy)return true;
  if(result!=Q::Queued)return false;
  return control_.queued(ticket,tx_.generation(),seq,epoch,now,ttl);
 }
 // A neutral-only sample may precede START/RESUME; never substitutes for
 // independent Boat lease validation. No nonzero override in this step.
 bool sendNeutral(uint32_t now,const MissionControlAuthority& a,bool gap,uint8_t epoch,uint16_t& seq,uint16_t rekeyAt,const uint8_t key[32]){
  using L=typename Tx::Link;using Q=typename Tx::Submit;using S=AsyncMissionControl::State;
  if(!neutralAllowed(a)||!control_.transaction()||!control_.fresh(now)||
     (control_.phase()!=MC_PREPARED&&control_.phase()!=MC_PAUSED&&control_.phase()!=MC_OVERRIDE_NEUTRAL))return false;
  if(control_.state()==S::Unknown||control_.inFlight())return false;
  if(neutralTicket_||!gap||tx_.pending(L::Primary)||(neutralOffered_&&uint32_t(now-neutralAt_)<50U))return true;
  if(!epoch||!key||rekeyAt<2||seq>=uint16_t(rekeyAt-1)||tx_.linkFault(L::Primary))return false;
  RadioFrame f{};radio_frameInit(f,RADIO_MSG_MISSION_STICKS,1,2,uint16_t(seq+1),control_.transaction(),now,epoch);
  if(!missionStickEncode({control_.transaction(),0,0,true},f.payload)||!scopedMissionFrame(f)||!radio_frameSeal(f,key))return false;
  ++seq;uint32_t ticket=0;const auto result=tx_.submitMission(f,now,NeutralLifetime,now,ticket,a.missionAuthority);
  if(result==Q::Busy)return true;
  if(result!=Q::Queued)return false;
  neutralTicket_=ticket;neutralGeneration_=tx_.generation();neutralSequence_=seq;neutralEpoch_=epoch;neutralAt_=now;neutralOffered_=true;return neutralEvidence_.offered(control_.transaction(),seq,epoch,neutralGeneration_,now);
 }
 bool observeCompletion(const typename Tx::Result& c){
  using L=typename Tx::Link;using K=typename Tx::Kind;
  if(c.link!=L::Primary||c.kind!=K::Mission||c.stop)return false;
  if(c.messageType==RADIO_MSG_MISSION_CONTROL)
   return control_.complete(c.ticket,c.generation,c.sequence,c.epoch,c.commandId,c.createdMs,c.lifetimeMs);
  if(c.messageType!=RADIO_MSG_MISSION_STICKS||!neutralTicket_||c.ticket!=neutralTicket_||
     c.generation!=neutralGeneration_||c.sequence!=neutralSequence_||c.epoch!=neutralEpoch_||
     c.commandId!=control_.transaction()||c.createdMs!=neutralAt_||c.lifetimeMs!=NeutralLifetime)return false;
  neutralTicket_=0;return true; // no peer lease claim
 }
 void transportRevoked(){neutralTicket_=0;neutralOffered_=false;neutralEvidence_.reset();control_.transportRevoked();}
 void resetAfterSession(){neutralTicket_=0;neutralOffered_=false;neutralEvidence_.reset();control_.resetAfterSession();}
private:
 static constexpr uint32_t NeutralLifetime=50U;
 static bool neutralAllowed(const MissionControlAuthority& a){
  return a.session&&a.primaryFresh&&a.missionAuthority&&a.stopReleased&&a.neutralLocal&&a.stationary&&
   a.actuatorReady&&a.batteryOk&&!a.failsafe;
 }
 Tx& tx_;AsyncMissionControl control_;RemoteNeutralEvidence neutralEvidence_;
 uint32_t neutralTicket_=0,neutralGeneration_=0,neutralAt_=0;uint16_t neutralSequence_=0;uint8_t neutralEpoch_=0;
 bool neutralOffered_=false;
};
}
