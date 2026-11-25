#pragma once
#include "remote_async_handshake.h"
#include "remote_tx_router.h"
#include "remote_clear_gate.h"
#include "remote_async_ordinary.h"
#include "remote_async_control.h"
#include "radio_ack_tracker.h"
#include "mission_wire.h"
#include "mission_control_wire.h"
#include "remote_mission_prepare_tx.h"
#include "remote_primary_recovery_gate.h"
// R30H-124J: authenticated application composition for isolated mock runtime.
// Isolated mock only. No physical driver, real joystick, or mission authority.
namespace flytec_rtos {
class RemoteAuthenticatedMockApp {
public:
 RemoteAuthenticatedMockApp(RemoteRadioOwner& owner,RadioFrameHandoff& handoff,RemoteAsyncTx& tx):
  owner_(owner),handoff_(handoff),tx_(tx),handshake_(owner,tx,session_),control_(owner,tx),ordinary_(tx),mission_(tx),missionControl_(tx){}
 RemoteAuthenticatedMockApp(const RemoteAuthenticatedMockApp&)=delete;
 RemoteAuthenticatedMockApp& operator=(const RemoteAuthenticatedMockApp&)=delete;
 bool initialize(const uint8_t root[32],SessionHmac mac){
  if(!owner_.worker()||initialized_||!root||!mac)return false;
  uint8_t any=0;for(unsigned i=0;i<32;++i)any|=root[i];if(!any)return false;
  memcpy(root_,root,32);mac_=mac;initialized_=true;
  return reset(0);
 }
 bool established()const{return owner_.worker()&&initialized_&&session_.established();}
 bool confirmed()const{return established()&&control_.confirmed();}
 bool backupConfirmed()const{return established()&&control_.backupConfirmed();}
 bool primaryFault()const{return primaryFault_;}
 bool backupFault()const{return backupFault_;}
 // Authenticated backup snapshot, not a primary movement permission.
 static constexpr uint32_t BackupStatusAge=6500U;
 bool backupStatusFresh(uint32_t now)const{return established()&&!backupFault_&&backupStatusSeen_&&uint32_t(now-backupStatusAt_)<BackupStatusAge;}
 bool backupStatus(uint32_t now,RadioDecodedPayload& out)const{if(!backupStatusFresh(now))return false;out=backupStatus_;return true;}
 // Backup-only requests are explicit and separate from primary control.
 bool backupMissionBound()const{return established()&&backupScope_;}
 BackupMissionControl::State backupMissionState()const{return backupMission_.state();}
 uint16_t backupMissionHighWater()const{return backupMission_.requestHighWater();}
 // Historical authenticated result, never an authorization or live phase.
 bool lastBackupMissionFeedback(MissionControlFeedback& out)const{
  if(!established()||!backupResultSeen_)return false;
  out=backupResult_;return true;
 }
 bool requestBackupMissionControl(MissionControlOp op,uint32_t now){
  // A probe already admitted to the primary lane must settle before a
  // new backup transaction. Shared replay ordering cannot tolerate overlap.
  return backupMissionBound()&&!probeTicket_&&!recovery_.awaiting(now)&&backupMission_.request(op,now,backupAuthority(now));
 }
 bool requestStop(uint16_t id,uint32_t now){
  if(!established()||!id)return false;
  if(id==stopId_)return control_.hasStopIntent(id)&&!rearmed_&&!clear_.pending()&&!missionLocked();
  // STOP is always prioritized, including when one lane failed.
  BoatCommand c{};c.type=CMD_EMERGENCY_STOP;c.mode=MODE_MANUAL;c.commandId=id;
  if(!control_.requestStop(c,now))return false;
  clear_.reset();clearTicket_=0;ordinary_.intent().transportRevoked();rearmed_=false;
  if(missionLocked())mission_.upload().transportRevoked();
  missionControl_.transportRevoked();revokeBackupMission();resetRecovery();missionClear_=missionAuthorized_=false;
  if(!ack_.begin(id,CMD_EMERGENCY_STOP,MODE_MANUAL,now,15000U)){reset(now);return false;}
  stopId_=id;return true;
 }
 // Explicit simulated input, never a substitute for real calibrated sticks.
 void mockInput(bool valid,bool neutral){if(owner_.worker()){inputValid_=valid;inputNeutral_=neutral;if(!valid||!neutral)missionControl_.invalidateNeutral();}}
 bool rearmed()const{return established()&&rearmed_;}
 bool commandAck(uint16_t id,RadioCommandAckRecord& out)const{return owner_.worker()&&ack_.get(id,out);}
 bool requestClear(uint16_t id,uint32_t now){return requestClearInternal(id,now,false);}
 bool requestMissionClear(uint16_t id,uint32_t now){return requestClearInternal(id,now,true);}
private:
 bool requestClearInternal(uint16_t id,uint32_t now,bool mission){
  RadioCommandAckRecord old{};
  if(!established()||!id||rearmed_||(mission?(!missionHandedOff()||missionAuthorized_||uint32_t(now-mission_.upload().started())>=9000U):missionLocked())||!control_.confirmed()||!control_.ordinaryGap(now)||
     !primaryStatusSeen_||primaryStatus_.stopState!=1||ack_.get(id,old)||sequence_>=RadioSessionCounter::REKEY_AT-1)return false;
  const auto auth=clearAuth(now);
  if(!clear_.begin(id,1,now,true,auth))return false;
  BoatCommand cmd{};cmd.type=CMD_CLEAR_STOP;cmd.mode=MODE_MANUAL;cmd.commandId=id;cmd.sequence=++sequence_;cmd.createdAtMs=now;
  RadioFrame f{};
  if(!radio_encodeCommand(cmd,1,2,1,f,key_)){clear_.reset();return false;}
  const auto result=mission?tx_.submitMissionClear(mission_.upload().transaction(),f,now,100U,now,clearTicket_,true):tx_.submitClear(f,now,100U,now,clearTicket_,true);
  if(result!=RemoteAsyncTx::Submit::Queued){clear_.reset();return false;}
  if(!ack_.begin(id,CMD_CLEAR_STOP,MODE_MANUAL,now,RemoteClearGate::Deadline)){inhibit(now);return false;}
  missionClear_=mission;return true;
 }
public:
 bool requestOrdinary(const BoatCommand& cmd,uint32_t now){
  if(!rearmed()||clear_.pending()||missionLocked())return false;
  RadioCommandAckRecord old{};
  if(cmd.type!=CMD_MANUAL_MOTORS&&ack_.get(uint16_t(cmd.commandId),old))return false;
  return ordinary_.intent().stage(cmd,ordinaryAuth(now),now);
 }
 // Mock-only preparation. Battery percentage here is simulated telemetry;
 // real production must also check fresh measured battery voltage.
 bool requestMission(const MissionPlan& plan,uint32_t now){
  return owner_.worker()&&!missionLocked()&&missionSafe(now)&&mission_.upload().begin(plan,1,now,true);
 }
 bool missionPrepared(uint32_t now)const{return owner_.worker()&&missionSafe(now)&&mission_.upload().prepared(now);}
 // Explicit handoff binds the transaction to future control; no START or CLEAR.
 bool acceptPreparedHandoff(uint32_t now){
  if(!owner_.worker()||!missionSafe(now)||!preparedStatusSeen_||tx_.pending(PhyLink::Primary)||
     !mission_.upload().handoff(preparedStatusId_,1,preparedStatusAt_,now,true))return false;
  if(!tx_.handoffMission(preparedStatusId_,true)||
     !missionControl_.control().bind(preparedStatusId_,1,mission_.upload().started(),preparedStatusAt_,now,true)){
   inhibit(now);return false;
  }
  return true;
 }
 bool requestMissionCancel(uint32_t now){
  return owner_.worker()&&!missionClear_&&!missionAuthorized_&&mission_.upload().beginCancel(now,missionSafe(now));
 }
 bool missionAuthorized()const{return established()&&missionAuthorized_;}
 bool missionNeutral(uint32_t now)const{return missionControl_.neutralFresh(1,now,inputValid_&&inputNeutral_);}
 // Last observed phase is historical when missionOutcomeUnknown() is true.
 bool missionOutcomeUnknown()const{return missionControl_.control().state()==AsyncMissionControl::State::Unknown;}
 MissionControlPhase missionPhase()const{return missionControl_.control().phase();}
 bool requestMissionControl(MissionControlOp op,uint32_t now){
  return missionAuthorized()&&missionControl_.control().request(op,now,missionAuthority(now));
 }
 bool missionHandedOff()const{return mission_.upload().state()==AsyncMissionUpload::State::HandedOff;}
 bool missionCancelled()const{return mission_.upload().state()==AsyncMissionUpload::State::Cancelled&&!mission_.upload().inFlight();}
 bool missionLocked()const{return mission_.upload().state()!=AsyncMissionUpload::State::Idle;}
 // Local cancellation only. Peer may have retained a preparation: keep
 // CLEAR and movement blocked until a fresh session or future handoff.
 bool cancelMissionLocal(uint32_t now){return owner_.worker()&&missionLocked()&&inhibit(now);}
 bool tick(uint32_t now,uint64_t nonce,bool primaryFault,bool backupFault){
  if(!owner_.worker()||!initialized_)return false;
  const bool primaryUnavailableNow=primaryFault||tx_.linkFault(PhyLink::Primary);
  primaryFault_|=primaryUnavailableNow;
  recoveryAllowed_=primaryFault_&&!primaryUnavailableNow;
  if(!recoveryAllowed_){recovery_.reset();recoveryGeneration_=0;}
  backupFault_|=backupFault||tx_.linkFault(PhyLink::Backup);
  if(primaryFault_&&backupFault_){retire(now);return false;}
  if((session_.established()&&sequence_>=RadioSessionCounter::REKEY_AT-1)||session_.timeout(now)){
   if(!reset(now))return false;
  }
  if(missionAuthorized_&&(!missionAuthority(now).primaryFresh||
     !missionAuthority(now).stopReleased||!missionAuthority(now).actuatorReady||
     !missionControl_.control().maintain(now,missionAuthority(now)))){
   // Only explicit primary fault transfers a previously authorized scope.
   // Otherwise (unsafe/stale primary alone) retain the existing inhibit path.
   if(primaryFault_&&!backupFault_){if(!transferBackupMission(now)&&!inhibit(now))return reset(now);}
   else if(!inhibit(now))return reset(now);
  }
  consume(PhyLink::Primary,now);consume(PhyLink::Backup,now);
  if(!initialized_)return false;
  // Recheck after RX: a fresh unsafe STATUS must revoke in this same pass.
  if(missionAuthorized_&&(!missionAuthority(now).primaryFresh||!missionAuthority(now).stopReleased||!missionAuthority(now).actuatorReady)){
   if(!inhibit(now))return reset(now);
  }
  // Await fresh post-transfer evidence for a bounded interval. Once seen,
  // stale/unsafe STATUS retires the entire generation before new admission.
  if(backupScope_&&(backupFault_||
     (backupEvidenceSeen_&&!backupSafety(now))||
     ((!backupEvidenceSeen_||!backupPhaseSeen_)&&uint32_t(now-backupTransferAt_)>=BackupStatusAge)||
     (backupPhaseSeen_&&(uint32_t(now-backupPhaseAt_)>=BackupStatusAge||backupPhase_==MC_RETURNING||backupPhase_==MC_RELEASING||
       backupPhase_==MC_OVERRIDE_ACTIVE||backupPhase_==MC_OVERRIDE_NEUTRAL||backupPhase_==MC_MANUAL_REQUESTED))||
     !backupMission_.maintain(now,backupAuthority(now)))){
   if(!inhibit(now))return reset(now);
  }
  // Fail closed before draining results when authority was withdrawn.
  if(!missionClear_&&!missionAuthorized_&&(missionActive()||mission_.upload().inFlight())&&!mission_.upload().maintain(now,missionSafe(now))){if(!inhibit(now))return reset(now);}
  if(mission_.upload().state()==AsyncMissionUpload::State::Failed&&mission_.upload().inFlight()){if(!inhibit(now))return reset(now);}
  if(clear_.pending()||clear_.state()==RemoteClearGate::State::Failed){
   if((!clear_.maintain(now,clearAuth(now))||(missionClear_&&uint32_t(now-mission_.upload().started())>=9000U))&&!inhibit(now))return reset(now);
  }
  if(rearmed_&&!ordinaryAuth(now).actuatorReady){if(!inhibit(now))return reset(now);}
  if(ordinary_.intent().state()==OrdinaryIntent::State::InFlight&&!ordinary_.intent().stillAllowed(ordinaryAuth(now),now)){
   if(!inhibit(now))return reset(now);
  }
  if(!routeRemoteTx(tx_,now,
      [&](const RemoteAsyncTx::Result& c,uint32_t at){return handshake_.observeCompletion(c,at);},
      [&](const RemoteAsyncTx::Result& c,uint32_t at){
       if(c.messageType==RADIO_MSG_LINK_PROBE){
        if(!probeTicket_||c.ticket!=probeTicket_||c.link!=PhyLink::Primary||c.epoch!=1||c.sequence!=probeSequence_||c.generation!=tx_.generation())return false;
        probeTicket_=0;return true; // neither local ACK nor failure qualifies recovery
       }
       return control_.observeCompletion(c,at);
      },
      [&](const RemoteAsyncTx::Result& c,uint32_t at){
       if(clearTicket_&&c.ticket==clearTicket_&&c.commandId==clear_.id()&&c.link==PhyLink::Primary&&c.epoch==1&&c.messageType==RADIO_MSG_ACTION){clearTicket_=0;return true;}
       return ordinary_.observeCompletion(c,at);
      },[&](const RemoteAsyncTx::Result& c,uint32_t at){
       if(c.link==PhyLink::Backup)return control_.observeBackupMissionCompletion(backupMission_,c,at);
       return c.messageType==RADIO_MSG_MISSION_PREPARE?mission_.observeCompletion(c):missionControl_.observeCompletion(c);
      }))return reset(now);
  if(!session_.established()){
   if(!handshake_.active()&&!handshake_.start(nonce,now,SessionHandshake::windowFor(!backupFault_)))return false;
   return handshake_.tick(now,!primaryFault_,!backupFault_,root_,true);
  }
  ack_.update(now);
  // Do not allow a busy ordinary/CLEAR slot to delay primary heartbeat.
  // The shared generation revocation keeps the STOP latch closed and
  // preserves backup cooldown; already admitted RF is not retractable.
  if(!primaryFault_&&control_.ordinaryBlockingHeartbeat(now)){
   if(!inhibit(now))return reset(now);
  }
  if(clear_.ready(now,clearAuth(now))&&primaryStatus_.stopState==0&&(primaryStatus_.safetyFlags&1U)){
   if(missionClear_){
    if(!tx_.pending(PhyLink::Primary)&&!clearTicket_){
     if(!control_.confirmMissionClear(mission_.upload().transaction(),clear_.id(),true))return reset(now);
     clear_.reset();missionClear_=false;missionAuthorized_=true;
    }
   }else{
    if(!control_.rearmAfterClear(true,now))return reset(now);
    clear_.reset();clearTicket_=0;rearmed_=true;
   }
  }
  // Explicit pending backup mission traffic shares pacing with heartbeat;
  // STOP always revokes the scope and is serviced by control_.tick.
  if(backupScope_&&!control_.serviceBackupMission(backupMission_,backupAuthority(now),now,sequence_,RadioSessionCounter::REKEY_AT,key_)){
   if(!inhibit(now))return reset(now);
  }
  if(recoveryAllowed_&&!servicePrimaryRecovery(now))return reset(now);
  const uint8_t flags=uint8_t(1U|(!primaryFault_?2U:0U)|(!backupFault_?4U:0U)|(control_.confirmed()?8U:0U));
  if(!control_.tick(now,!primaryFault_,!backupFault_,flags,sequence_,RadioSessionCounter::REKEY_AT,key_,true))
   return reset(now);
  if(!missionClear_&&!missionAuthorized_&&missionActive()&&!mission_.service(now,missionSafe(now),control_.ordinaryGap(now),1,sequence_,RadioSessionCounter::REKEY_AT,key_)){
   if(!inhibit(now))return reset(now);
  }
  if(missionAuthorized_){
   const auto a=missionAuthority(now);
   if(!missionControl_.service(now,a,control_.ordinaryGap(now),1,sequence_,RadioSessionCounter::REKEY_AT,key_)){
    if(!inhibit(now))return reset(now);
   }else if(a.neutralLocal&&!a.neutralLease&&a.stationary&&
     (missionPhase()==MC_PREPARED||missionPhase()==MC_PAUSED||missionPhase()==MC_OVERRIDE_NEUTRAL)&&
     control_.ordinaryGap(now)){
    if(!missionControl_.sendNeutral(now,a,true,1,sequence_,RadioSessionCounter::REKEY_AT,key_)&&!inhibit(now))return reset(now);
   }
  }
  if(rearmed_){
   const auto previous=ordinary_.intent().state();
   if(!ordinary_.service(ordinaryAuth(now),now,control_.ordinaryGap(now),1,sequence_,RadioSessionCounter::REKEY_AT,key_,radio_encodeCommand)){
    if(!inhibit(now))return reset(now);
   }
   if(previous==OrdinaryIntent::State::Staged&&ordinary_.intent().state()==OrdinaryIntent::State::InFlight){
    const auto& c=ordinary_.intent().command();
    if(c.type!=CMD_MANUAL_MOTORS&&!ack_.begin(uint16_t(c.commandId),uint8_t(c.type),uint8_t(c.mode),c.createdAtMs,3500U)){
     if(!inhibit(now))return reset(now);
    }
   }
  }
  return true;
 }
 void retire(uint32_t now){
  if(!owner_.worker()||!initialized_)return;
  reset(now);radioSession_wipe(root_,32);initialized_=false;mac_=nullptr;
 }
private:
 void resetRecovery(){recovery_.reset();recoveryGeneration_=0;probeTicket_=0;probeSequence_=0;}
 bool servicePrimaryRecovery(uint32_t now){
  if(!established()||!primaryFault_||tx_.linkFault(PhyLink::Primary))return false;
  const auto generation=tx_.generation();
  if(recoveryGeneration_!=generation){
   if(!recovery_.begin(1,generation))return false;
   recoveryGeneration_=generation;lastProbeAt_=now-500U;
  }
  recovery_.expire(now);
  // Avoid consuming a newer primary echo before an older LoRa mission reply.
  // Backup operations keep their existing absolute deadline and STOP priority.
  if(backupMission_.inFlight()||backupMission_.state()==BackupMissionControl::State::Pending){
   recovery_.begin(1,generation);return true;
  }
  // Never revoke an ambiguous PAUSE/CANCEL just because primary returns.
  if(recovery_.ready(now,1,generation)&&!probeTicket_&&!tx_.pending(PhyLink::Primary)&&
     !backupMission_.inFlight()&&backupMission_.state()!=BackupMissionControl::State::Pending){
   // Preserve any existing STOP intent/ACK tracker; otherwise revoke the old
   // backup scope and retain the historical peer phase without resuming it.
   if(!control_.hasStopIntent(stopId_)&&!inhibit(now))return false;
   primaryFault_=false;primaryStatusSeen_=false;recoveryAllowed_=false;resetRecovery();
   return true;
  }
  if(probeTicket_||tx_.pending(PhyLink::Primary)||uint32_t(now-lastProbeAt_)<500U)return true;
  if(sequence_>=RadioSessionCounter::REKEY_AT-1)return false;
  RadioFrame f{};radio_frameInit(f,RADIO_MSG_LINK_PROBE,1,2,uint16_t(sequence_+1),0,now,1);
  if(!recoveryProbeEncode(RecoveryProbeKind::Challenge,f.sequence,f.payload)||!radio_frameSeal(f,key_))return false;
  ++sequence_;uint32_t ticket=0;
  const auto result=tx_.submitRecoveryProbe(f,now,ticket,true);
  if(result==RemoteAsyncTx::Submit::Busy)return true;
  if(result!=RemoteAsyncTx::Submit::Queued)return false;
  probeTicket_=ticket;probeSequence_=f.sequence;lastProbeAt_=now;
  return recovery_.offer(f.sequence,now,1,generation);
 }
 bool backupSafety(uint32_t now)const{
  return backupEvidenceSeen_&&backupStatusFresh(now)&&backupStatus_.stopState==0&&
   backupStatus_.batteryPercent>20&&backupStatus_.batteryPercent<=100&&
   (backupStatus_.safetyFlags&0xE0U)==0&&(backupStatus_.safetyFlags&1U);
 }
 BackupMissionAuthority backupAuthority(uint32_t now)const{
  const bool known=backupPhaseSeen_&&uint32_t(now-backupPhaseAt_)<BackupStatusAge&&
   (backupPhase_==MC_OUTBOUND||backupPhase_==MC_PAUSED);
  return {established(),primaryFault_,backupSafety(now),backupScope_&&known,
   backupStatus_.batteryPercent>20&&backupStatus_.batteryPercent<=100,
   (backupStatus_.safetyFlags&0xE0U)!=0,!backupScope_};
 }
 bool transferBackupMission(uint32_t now){
  const auto id=missionControl_.control().transaction();
  const auto high=missionControl_.control().requestHighWater();
  if(!id||!control_.transferMissionToBackup(id,now,true))return false;
  missionControl_.transportRevoked();mission_.upload().transportRevoked();ordinary_.intent().transportRevoked();
  missionClear_=missionAuthorized_=false;clear_.reset();clearTicket_=0;rearmed_=false;
  resetRecovery();backupTransferAt_=now;backupEvidenceSeen_=backupPhaseSeen_=false;
  backupScope_=backupMission_.bind(id,1,tx_.generation(),high,true);
  return backupScope_;
 }
 void revokeBackupMission(){backupMission_.transportRevoked();backupScope_=backupEvidenceSeen_=backupPhaseSeen_=false;}
 MissionControlAuthority missionAuthority(uint32_t now)const{
  MissionControlAuthority a{};
  a.session=established();a.primaryFresh=primaryStatusFresh(now);a.missionAuthority=missionAuthorized_;
  a.stopReleased=primaryStatus_.stopState==0;a.neutralLocal=inputValid_&&inputNeutral_;
  a.neutralLease=missionNeutral(now);a.stationary=(primaryStatus_.safetyFlags&6U)==0;
  a.batteryOk=primaryStatus_.batteryPercent>20&&primaryStatus_.batteryPercent<=100;
  a.failsafe=(primaryStatus_.safetyFlags&0xE0U)!=0;
  a.actuatorReady=a.session&&a.primaryFresh&&a.stopReleased&&a.batteryOk&&!a.failsafe&&(primaryStatus_.safetyFlags&1U);
  return a;
 }
 bool missionActive()const{
  const auto s=mission_.upload().state();return s==AsyncMissionUpload::State::Active||s==AsyncMissionUpload::State::Prepared||s==AsyncMissionUpload::State::HandedOff||s==AsyncMissionUpload::State::Canceling;
 }
 bool missionSafe(uint32_t now)const{
  return established()&&control_.confirmed()&&!rearmed_&&!clear_.pending()&&primaryStatusFresh(now)&&
   primarySafe()&&primaryStatus_.stopState==1&&(primaryStatus_.safetyFlags&6U)==0&&
   primaryStatus_.batteryPercent>20&&primaryStatus_.batteryPercent<=100&&inputValid_&&inputNeutral_;
 }
 bool primaryStatusFresh(uint32_t now)const{return !primaryFault_&&primaryStatusSeen_&&uint32_t(now-primaryStatusAt_)<250U;}
 bool primarySafe()const{return primaryStatus_.boatMode==MODE_MANUAL&&(primaryStatus_.safetyFlags&0xE0U)==0;}
 ClearAuthorization clearAuth(uint32_t now)const{
  return {established(),primaryStatusFresh(now),inputValid_&&inputNeutral_,
   (primaryStatus_.safetyFlags&6U)==0,primarySafe()};
 }
 OrdinaryAuthorization ordinaryAuth(uint32_t now)const{
  OrdinaryAuthorization a{};
  a.sessionReady=established();a.primaryFresh=primaryStatusFresh(now);a.stopConfirmed=rearmed_;
  a.stopped=primaryStatus_.stopState!=0;a.manualConfirmed=primaryStatus_.boatMode==MODE_MANUAL;
  a.joystickValid=inputValid_;a.missionLocked=missionLocked();a.failsafe=!primarySafe();
  a.actuatorReady=a.primaryFresh&&a.sessionReady&&rearmed_&&!a.stopped&&!a.failsafe&&(primaryStatus_.safetyFlags&1U);
  return a;
 }
 bool inhibit(uint32_t now){
  const bool ok=control_.inhibitOrdinary(now);
  if(ok){ordinary_.intent().transportRevoked();if(missionLocked())mission_.upload().transportRevoked();missionControl_.transportRevoked();}
  revokeBackupMission();resetRecovery();missionClear_=missionAuthorized_=false;
  clear_.reset();clearTicket_=0;rearmed_=false;
  return ok;
 }
 bool reset(uint32_t now){
  const bool defer=control_.backupRecentlyUsed(now);
  const bool a=control_.invalidate(now);
  const bool b=handshake_.reset(now,defer);
  radioSession_wipe(key_,32);sequence_=stopId_=0;ack_.reset(0);
  clear_.reset();clearTicket_=0;ordinary_.intent().reset();rearmed_=false;primaryStatusSeen_=backupStatusSeen_=false;backupStatusAt_=0;backupStatus_=RadioDecodedPayload{};inputValid_=inputNeutral_=false;
  if(a&&b){mission_.upload().resetAfterSession();missionControl_.resetAfterSession();backupMission_.resetAfterSession();}
  backupScope_=backupEvidenceSeen_=backupPhaseSeen_=false;backupResultSeen_=false;backupResult_=MissionControlFeedback{};resetRecovery();recoveryAllowed_=false;
  missionClear_=missionAuthorized_=false;
  preparedStatusSeen_=false;preparedStatusId_=0;preparedStatusAt_=0;
  cross_.resetAfterHandshake();primary_.resetAfterHandshake();backup_.resetAfterHandshake();
  return a&&b;
 }
 void consume(PhyLink link,uint32_t now){
  auto& queue=link==PhyLink::Primary?handoff_.primaryRx:handoff_.backupRx;
  RxFrame rx{};
  for(unsigned i=0;i<4&&queue.take(rx);++i){
   if(rx.link!=link||!remoteRxFresh(now,rx.receivedMs))continue;
   const RadioFrame& f=rx.frame;
   if(link==PhyLink::Backup?backupFault_:(primaryFault_&&(!recoveryAllowed_||f.type!=RADIO_MSG_LINK_PROBE)))continue;
   if(f.sourceId!=2||f.destinationId!=1||!radioLinkFlagMatches(f.flags,link==PhyLink::Backup))continue;
   if(f.type>=RADIO_MSG_SESSION_HELLO&&f.type<=RADIO_MSG_SESSION_CONFIRM){
    if(handshake_.receive(link,f,rx.receivedMs,now,root_,mac_)&&session_.established()){
     if(!session_.frameKey(key_)||!control_.beginSession(1,now)){retire(now);return;}
     if(handshake_.backupRecentlyUsed(now))control_.deferBackup(now);
     ack_.reset(1);
    }
    continue;
   }
   if(!session_.established()||f.sessionEpoch!=1||!radio_frameVerify(f,key_))continue;
   if(f.type==RADIO_MSG_LINK_PROBE){
    uint16_t echoed=0;
    if(link==PhyLink::Primary&&primaryFault_&&recoveryAllowed_&&!f.commandId&&
       !backupMission_.inFlight()&&backupMission_.state()!=BackupMissionControl::State::Pending&&
       recoveryProbeDecode(f.payload,RecoveryProbeKind::Echo,echoed)&&
       recovery_.matchesEcho(echoed,rx.receivedMs,now,1,tx_.generation())&&
       primary_.accept(f.sequence)&&cross_.accept(f.sequence))
     recovery_.observe(echoed,rx.receivedMs,now,1,tx_.generation(),true);
    continue;
   }
   if(f.type==RADIO_MSG_MISSION_NEUTRAL_ACK){
    if(link!=PhyLink::Primary||!missionAuthorized_||!primary_.accept(f.sequence)||!cross_.accept(f.sequence))continue;
    missionControl_.observeNeutralReceipt(f,rx.receivedMs,now,true);continue;
   }
   if(f.type==RADIO_MSG_MISSION_FEEDBACK){
    MissionControlFeedback feedback{};
    if(!missionFeedbackDecode(f.payload,feedback)||feedback.transaction!=f.commandId||
       feedback.transaction!=mission_.upload().transaction())continue;
    if(link==PhyLink::Backup){
     if(!backupScope_||feedback.transaction!=backupMission_.transaction()||
        uint32_t(rx.receivedMs-backupTransferAt_)>=0x80000000U||
        !backup_.accept(f.sequence)||!cross_.accept(f.sequence))continue;
     if(feedback.result==MC_STATUS){
      backupPhase_=feedback.phase;backupPhaseAt_=rx.receivedMs;backupPhaseSeen_=true;
     }else if(backupMission_.observe(feedback,1,tx_.generation(),rx.receivedMs,now,true,backupAuthority(now))){
      backupResult_=feedback;backupResultSeen_=true;
      backupPhase_=feedback.phase;backupPhaseAt_=rx.receivedMs;backupPhaseSeen_=true;
     }
     continue;
    }
    if(!primary_.accept(f.sequence)||!cross_.accept(f.sequence))continue;
    if(missionHandedOff())missionControl_.observeControlFeedback(feedback,1,rx.receivedMs,now,true);
    if(feedback.result!=MC_STATUS)continue;
    preparedStatusSeen_=feedback.phase==MC_PREPARED;
    preparedStatusId_=feedback.transaction;preparedStatusAt_=rx.receivedMs;continue;
   }
   if(f.type==RADIO_MSG_MISSION_ACK){
    uint16_t id=0;MissionOpcode op=MISSION_BEGIN;MissionReceive result=MISSION_REJECTED;
    if(link!=PhyLink::Primary||!missionAck_decode(f.payload,id,op,result)||id!=f.commandId||!missionSafe(now))continue;
    if(!primary_.accept(f.sequence)||!cross_.accept(f.sequence))continue;
    mission_.upload().acknowledge(id,op,result,f.sessionEpoch,rx.receivedMs,now,true);
    continue;
   }
   RadioDecodedPayload decoded{};
   if(f.type==RADIO_MSG_ACK){
    if(!radio_decodeAckPayload(f.payload,decoded)||f.commandId!=decoded.commandId)continue;
   }else if(f.type==RADIO_MSG_TELEMETRY){
    if(f.commandId||!radio_decodeTelemetryPayload(f.payload,decoded)||decoded.kind!=RADIO_PAYLOAD_STATUS)continue;
   }else if(f.type==RADIO_MSG_HEARTBEAT){
    if(f.commandId||!radio_decodeHeartbeatPayload(f.payload,decoded))continue;
   }else continue;
   auto& guard=link==PhyLink::Primary?primary_:backup_;
   if(!guard.accept(f.sequence)||!cross_.accept(f.sequence))continue;
   if(decoded.kind==RADIO_PAYLOAD_ACK&&ack_.observe(decoded,link==PhyLink::Primary?RADIO_ACK_LINK_PRIMARY:RADIO_ACK_LINK_BACKUP,1,rx.receivedMs)){
    if(decoded.commandType==CMD_EMERGENCY_STOP)control_.observeStopAck(link,decoded.commandId,decoded.ackResult==RADIO_ACK_ACCEPTED||decoded.ackResult==RADIO_ACK_EXECUTED,rx.receivedMs,now);
    else if(decoded.commandType==CMD_CLEAR_STOP)clear_.ack(decoded.commandId,1,link==PhyLink::Primary,decoded.ackResult==RADIO_ACK_EXECUTED,decoded.ackResult!=RADIO_ACK_ACCEPTED&&decoded.ackResult!=RADIO_ACK_EXECUTED,rx.receivedMs,now);
   }else if(decoded.kind==RADIO_PAYLOAD_STATUS){
    control_.observeStopStatus(link,decoded.stopState==1,rx.receivedMs,now);
    if(link==PhyLink::Primary){
     primaryStatus_=decoded;primaryStatusAt_=rx.receivedMs;primaryStatusSeen_=true;
     clear_.status(1,true,decoded.stopState==0,primarySafe()&&(decoded.safetyFlags&7U)==1U,rx.receivedMs,now);
    }else{
     backupStatus_=decoded;backupStatusAt_=rx.receivedMs;backupStatusSeen_=true;
     if(backupScope_&&uint32_t(rx.receivedMs-backupTransferAt_)<0x80000000U)backupEvidenceSeen_=true;
    }
   }
  }
 }
 RemoteRadioOwner& owner_;RadioFrameHandoff& handoff_;RemoteAsyncTx& tx_;
 SessionHandshake session_;RemoteAsyncHandshake handshake_;RemoteAsyncControl control_;
 RemoteClearGate clear_;RemoteAsyncOrdinary<RemoteAsyncTx> ordinary_;RemoteMissionPrepareTx<RemoteAsyncTx> mission_;RemoteMissionControlTx<RemoteAsyncTx> missionControl_;
 RadioDecodedPayload primaryStatus_{},backupStatus_{};uint32_t primaryStatusAt_=0,backupStatusAt_=0,clearTicket_=0;
 bool backupStatusSeen_=false;
 PrimaryRecoveryGate recovery_;
 uint32_t recoveryGeneration_=0,probeTicket_=0,lastProbeAt_=0;
 uint16_t probeSequence_=0;bool recoveryAllowed_=false;
 BackupMissionControl backupMission_;
 MissionControlFeedback backupResult_{};bool backupResultSeen_=false;
 MissionControlPhase backupPhase_=MC_IDLE;
 uint32_t backupTransferAt_=0,backupPhaseAt_=0;
 bool backupScope_=false,backupEvidenceSeen_=false,backupPhaseSeen_=false;
 uint32_t preparedStatusAt_=0;uint16_t preparedStatusId_=0;bool preparedStatusSeen_=false;
 bool primaryStatusSeen_=false,inputValid_=false,inputNeutral_=false,rearmed_=false;
 RadioAckTracker ack_;RadioSessionCounter primary_,backup_,cross_;
 uint8_t root_[32]{},key_[32]{};SessionHmac mac_=nullptr;
 uint16_t sequence_=0,stopId_=0;
 bool missionClear_=false,missionAuthorized_=false;
 bool initialized_=false,primaryFault_=false,backupFault_=false;
};
}
