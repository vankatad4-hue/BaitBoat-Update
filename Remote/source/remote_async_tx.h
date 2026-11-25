#pragma once
#include "radio_frame_handoff.h"
#include "tx_generation_gate.h"
#include "remote_radio_owner.h"
#include "boat_commands.h"
#include "remote_mission_prepare_tx.h"
#include "remote_mission_tx_scope.h"
#include "remote_mission_control_tx.h"
#include "remote_backup_mission_control.h"
// R30H-124F experimental Core 1 application seam. Sole generation owner.
// Frames must already be authenticated/sealed by the application. Never
// interpret queue admission, nRF ACK or E22 local readiness as Boat STOP.
namespace flytec_rtos {
class RemoteAsyncTx {
public:
 using Link=PhyLink;
 enum class Kind:uint8_t {Unsupported,Handshake,Stop,Heartbeat,Ordinary,Mission};
 enum class Submit:uint8_t {Queued,Busy,Denied,Expired,Fault};
 enum class Outcome:uint8_t {Failed,NrfAck,LocalReady,Expired};
 struct Result {
  uint32_t ticket=0,generation=0;
  uint16_t commandId=0;
  PhyLink link=PhyLink::Primary;
  Outcome outcome=Outcome::Failed;
  bool stop=false;
  Kind kind=Kind::Unsupported;
  uint8_t messageType=0,epoch=0;
  uint16_t sequence=0;
  uint32_t createdMs=0,lifetimeMs=0;
 };
 RemoteAsyncTx(RemoteRadioOwner& o,RadioFrameHandoff& h,TxGenerationGate& g)
 :owner_(o),handoff_(h),gate_(g){}
 RemoteAsyncTx(const RemoteAsyncTx&)=delete;
 RemoteAsyncTx& operator=(const RemoteAsyncTx&)=delete;
 // Call on EVERY key/session change, even when the wire epoch is reused.
 // A failed transition leaves submissions inhibited; retry before service.
 bool invalidateSession(){
  if(!owner_.worker())return false;
  active_=false;epoch_=0;stopId_=0;clearSlots();
  return advanceStop();
 }
 // Root-key handshake only: no session command or heartbeat is allowed.
 bool beginHandshake(){
  if(!invalidateSession())return false;
  active_=true;return true;
 }
 bool beginSession(uint8_t epoch){
  if(!invalidateSession()||!epoch)return false;
  epoch_=epoch;active_=true;return true;
 }
 // New logical STOP: invalidate old queued/dequeued work before admission.
 // Call once per new intent, not once per retry or per link.
 bool requestStop(uint16_t commandId){
  if(!owner_.worker()||!active_||!epoch_||!commandId)return false;
  stopId_=0;clearSlots();
  if(!advanceStop()){active_=false;return false;}
  stopId_=commandId;return true;
 }
 // Confirmation cancels unadmitted retries but never authorizes movement.
 bool cancelStop(uint16_t commandId){
  if(!owner_.worker()||!active_||!stopId_||commandId!=stopId_)return false;
  stopId_=0;clearSlots();
  if(!advanceStop()){active_=false;return false;}
  return true;
 }
 // Only independent application authorization can rearm. TX results cannot.
 bool rearm(bool authorized){
  if(!owner_.worker()||!active_||!epoch_||!authorized||prepareUsed_||missionScope_.locked()||generation_==UINT32_MAX)return false;
  if(!gate_.rearm(generation_+1,true))return false;
  ++generation_;stopId_=0;clearSlots();return true;
 }
 // Keep the authenticated session for heartbeat/STOP, revoke both old TX
 // generations. This intentionally cancels backup work too; the owner must
 // preserve its conservative cooldown. No automatic rearm.
 bool inhibitSessionTraffic(){
  if(!owner_.worker()||!active_||!epoch_)return false;
  stopId_=0;clearSlots();
  if(!advanceStop()){active_=false;return false;}
  return true;
 }
 // Explicit narrow transfer, never from an idle/preparation/STOP scope.
 // Old primary and backup reservations are revoked before new authorization.
 bool transferMissionToBackup(uint16_t transaction,bool primaryUnavailable){
  if(!owner_.worker()||!active_||!epoch_||!gate_.stopped()||stopId_||
     !missionScope_.permitsMission(transaction,primaryUnavailable))return false;
  if(!inhibitSessionTraffic())return false;
  return missionScope_.openBackupAfterRevocation(transaction);
 }
 Submit submitBackupMission(const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t now,uint32_t& ticket,bool approved){
  ticket=0;MissionControlRequest q{};
  if(!owner_.worker()||!gate_.stopped()||stopId_||!missionScope_.permitsBackup(f.commandId,approved)||
     f.type!=RADIO_MSG_MISSION_CONTROL||f.sourceId!=1||f.destinationId!=2||f.flags!=RADIO_FLAG_BACKUP_LINK||
     !f.sequence||!missionControlDecode(f.payload,q)||q.transaction!=f.commandId||
     (q.op!=MCTRL_PAUSE&&q.op!=MCTRL_CANCEL)||ttl>BackupMissionControl::TxLifetime)return Submit::Denied;
  return submitImpl(PhyLink::Backup,f,created,ttl,now,ticket,false,false,true);
 }
 Submit submit(PhyLink link,const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t now,uint32_t& ticket){
  return submitImpl(link,f,created,ttl,now,ticket,false);
 }
 // One explicitly authorized, primary-only clear request per stopped
 // generation. Critical admission is scoped to this immutable zero-motion
 // frame; normal submit cannot bypass STOP or submit CLEAR_STOP.
 Submit submitClear(const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t now,uint32_t& ticket,bool approved){
  return submitClearImpl(f,created,ttl,now,ticket,approved,false);
 }
 bool handoffMission(uint16_t transaction,bool peerPrepared){
  return owner_.worker()&&active_&&epoch_&&gate_.stopped()&&!stopId_&&!clearUsed_&&
   missionScope_.handoff(transaction,peerPrepared,!primary_.used);
 }
 Submit submitMissionClear(uint16_t transaction,const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t now,uint32_t& ticket,bool approved){
  ticket=0;
  if(!owner_.worker()||!missionScope_.permitsClear(transaction,approved))return Submit::Denied;
  const auto result=submitClearImpl(f,created,ttl,now,ticket,approved,true);
  if(result==Submit::Queued)missionScope_.clearQueued(f.commandId);
  return result;
 }
 bool confirmMissionClear(uint16_t transaction,uint16_t command,bool peerEvidence){
  return owner_.worker()&&active_&&epoch_&&gate_.stopped()&&!stopId_&&clearUsed_&&
   missionScope_.confirmClear(transaction,command,peerEvidence,!primary_.used);
 }
 Submit submitMission(const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t now,uint32_t& ticket,bool approved){
  ticket=0;
  if(!owner_.worker()||!gate_.stopped()||stopId_||!scopedMissionFrame(f)||
     !missionScope_.permitsMission(f.commandId,approved)||ttl>100U||
     (f.type==RADIO_MSG_MISSION_STICKS&&ttl>50U))return Submit::Denied;
  return submitImpl(PhyLink::Primary,f,created,ttl,now,ticket,false,false,true);
 }
private:
 Submit submitClearImpl(const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t now,uint32_t& ticket,bool approved,bool mission){
  ticket=0;
  if(!owner_.worker()||!approved||!gate_.stopped()||stopId_||clearUsed_||(!mission&&(prepareUsed_||missionScope_.locked()))||
     f.type!=RADIO_MSG_ACTION||f.sourceId!=1||f.destinationId!=2||!f.sequence||!f.commandId||
     f.payload[0]!=CMD_CLEAR_STOP||ttl>100U)return Submit::Denied;
  for(unsigned i=1;i<8;++i)if(f.payload[i])return Submit::Denied;
  const auto result=submitImpl(PhyLink::Primary,f,created,ttl,now,ticket,true);
  if(result==Submit::Queued)clearUsed_=true;
  return result;
 }
public:
 // STOP-safe preparation only, never mission control or sticks. A prepared
 // generation cannot CLEAR/rearm until an explicit future handoff or revoke.
 Submit submitPrepare(const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t now,uint32_t& ticket,bool approved){
  ticket=0;
  if(!owner_.worker()||!approved||!gate_.stopped()||stopId_||clearUsed_||
     !(scopedPrepareFrame(f)||(prepareUsed_&&scopedCancelPreparationFrame(f)))||
     !missionScope_.permitsPrepare(f.commandId,f.payload[0]==MISSION_BEGIN,f.payload[0]==MISSION_CANCEL)||ttl>AsyncMissionUpload::TxLifetime)return Submit::Denied;
  const auto result=submitImpl(PhyLink::Primary,f,created,ttl,now,ticket,false,true);
  if(result==Submit::Queued){prepareUsed_=true;missionScope_.preparationQueued(f.commandId,f.payload[0]==MISSION_COMMIT,f.payload[0]==MISSION_CANCEL);}
  return result;
 }
private:
 Submit submitImpl(PhyLink link,const RadioFrame& f,uint32_t created,uint32_t ttl,uint32_t now,uint32_t& ticket,bool clear,bool prepare=false,bool mission=false){
  ticket=0;
  if(!owner_.worker()||!active_||!validLink(link)||f.sessionEpoch!=epoch_||
     !radioLinkFlagMatches(f.flags,link==PhyLink::Backup))return Submit::Denied;
  if(!ttl||ttl>INT32_MAX||uint32_t(now-created)>=ttl)return Submit::Expired;
  const bool handshake=(f.type==RADIO_MSG_SESSION_HELLO||f.type==RADIO_MSG_SESSION_PROOF)&&
   f.sequence==0&&f.commandId==0&&f.sessionEpoch==0;
  if(!epoch_&&!handshake)return Submit::Denied;
  if(epoch_&&f.type>=RADIO_MSG_SESSION_HELLO&&f.type<=RADIO_MSG_SESSION_CONFIRM)return Submit::Denied;
  const bool stop=f.type==RADIO_MSG_ACTION&&f.payload[0]==CMD_EMERGENCY_STOP;
  if(stop&&(!stopId_||f.commandId!=stopId_||!gate_.stopped()))return Submit::Denied;
  const bool safeHeartbeat=f.type==RADIO_MSG_HEARTBEAT&&f.commandId==0;
  if(f.type==RADIO_MSG_ACTION&&f.payload[0]==CMD_CLEAR_STOP&&!clear)return Submit::Denied;
  if(f.type==RADIO_MSG_MISSION_PREPARE&&!prepare)return Submit::Denied;
  if((f.type==RADIO_MSG_MISSION_CONTROL||f.type==RADIO_MSG_MISSION_STICKS)&&!mission)return Submit::Denied;
  const bool critical=stop||safeHeartbeat||handshake||clear||prepare||mission;
  if(gate_.stopped()&&!critical)return Submit::Denied;
  if(handoff_.fault(link)||nextTicket_==UINT32_MAX)return Submit::Fault;
  auto& s=slot(link);
  if(s.used)return Submit::Busy;
  TxRequest r{};r.frame=f;r.ticket=++nextTicket_;r.generation=generation_;
  r.link=link;r.createdMs=created;r.lifetimeMs=ttl;
  const bool queued=link==PhyLink::Primary?
   (critical?handoff_.primaryCritical.offer(r):handoff_.primaryTx.offer(r)):
   (critical?handoff_.backupCritical.offer(r):handoff_.backupTx.offer(r));
  if(!queued)return Submit::Fault;
  s.used=true;s.request=r;s.stop=stop;ticket=r.ticket;return Submit::Queued;
 }
public:
 uint32_t generation()const{return owner_.worker()?generation_:0;}
 bool pending(PhyLink link)const{return owner_.worker()&&validLink(link)&&(link==PhyLink::Primary?primary_.used:backup_.used);}
 bool linkFault(PhyLink link)const{return !owner_.worker()||!validLink(link)||handoff_.fault(link);}
 // Bounded drain. A late, duplicate, wrong-link or previous-generation
 // completion cannot retire the current request or become STOP evidence.
 bool take(PhyLink link,uint32_t now,Result& out){
  if(!owner_.worker()||!active_||!validLink(link))return false;
  auto& s=slot(link);
  TxCompletion c;
  for(unsigned i=0;i<8&&handoff_.takeCompletion(link,c);++i){
   if(!s.used||c.ticket!=s.request.ticket||c.generation!=generation_||
      c.generation!=s.request.generation||c.link!=link)continue;
   const auto outcome=expired(s,now)?Outcome::Expired:
    handoff_.fault(link)?Outcome::Failed:
    link==PhyLink::Primary&&c.success&&c.evidence==TxEvidence::NrfAck?Outcome::NrfAck:
    link==PhyLink::Backup&&!c.success&&c.evidence==TxEvidence::LocalReady?Outcome::LocalReady:Outcome::Failed;
   finish(s,out,outcome);return true;
  }
  if(s.used&&(expired(s,now)||handoff_.fault(link))){
   finish(s,out,expired(s,now)?Outcome::Expired:Outcome::Failed);return true;
  }
  return false;
 }
private:
 struct Slot{TxRequest request{};bool used=false,stop=false;};
 static bool validLink(PhyLink l){return l==PhyLink::Primary||l==PhyLink::Backup;}
 Slot& slot(PhyLink l){return l==PhyLink::Primary?primary_:backup_;}
 void clearSlots(){primary_=Slot{};backup_=Slot{};clearUsed_=false;prepareUsed_=false;missionScope_.resetAfterRevocation();}
 bool advanceStop(){
  if(generation_==UINT32_MAX){gate_.latchStop(generation_);return false;}
  if(!gate_.latchStop(generation_+1))return false;
  ++generation_;return true;
 }
 static bool expired(const Slot& s,uint32_t now){
  // The worker still enforces the original immutable admission TTL.
  // Only the application observation slot may outlive it for LoRa mission TX.
  const uint32_t grace=s.request.link==PhyLink::Backup&&s.request.frame.type==RADIO_MSG_MISSION_CONTROL
   ?BackupMissionControl::CompletionGrace:0U;
  return uint32_t(now-s.request.createdMs)>=s.request.lifetimeMs+grace;
 }
 static Kind classify(const RadioFrame& f,bool stop){
  if(stop)return Kind::Stop;
  switch(f.type){
   case RADIO_MSG_SESSION_HELLO:case RADIO_MSG_SESSION_PROOF:return Kind::Handshake;
   case RADIO_MSG_HEARTBEAT:return Kind::Heartbeat;
   case RADIO_MSG_ACTION:case RADIO_MSG_CONTROL:return Kind::Ordinary;
   case RADIO_MSG_MISSION_PREPARE:case RADIO_MSG_MISSION_CONTROL:case RADIO_MSG_MISSION_STICKS:return Kind::Mission;
   default:return Kind::Unsupported;
  }
 }
 static void finish(Slot& s,Result& out,Outcome outcome){
  out.ticket=s.request.ticket;out.generation=s.request.generation;
  out.commandId=s.request.frame.commandId;out.link=s.request.link;
  out.outcome=outcome;out.stop=s.stop;out.kind=classify(s.request.frame,s.stop);
  out.messageType=s.request.frame.type;out.epoch=s.request.frame.sessionEpoch;
  out.sequence=s.request.frame.sequence;out.createdMs=s.request.createdMs;out.lifetimeMs=s.request.lifetimeMs;s=Slot{};
 }
 RemoteRadioOwner& owner_;RadioFrameHandoff& handoff_;TxGenerationGate& gate_;
 Slot primary_,backup_;MissionTxScope missionScope_;
 uint32_t generation_=0,nextTicket_=0;
 uint16_t stopId_=0;uint8_t epoch_=0;bool active_=false,clearUsed_=false,prepareUsed_=false;
};
}
