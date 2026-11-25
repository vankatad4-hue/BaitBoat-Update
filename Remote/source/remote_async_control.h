#pragma once
#include "remote_async_tx.h"
#include "lora_link_policy.h"
#include "remote_backup_mission_control.h"
#include "radio_command_codec.h"
#include "remote_stop_delivery.h"
// R30H-124G: Core 1 application-side STOP/heartbeat scheduling.
// Use ONLY after authenticated session establishment. Caller owns key,
// session sequence counter, link readiness and authenticated RX dispatch.
// This module does not enable a physical worker or authorize movement.
namespace flytec_rtos {
class RemoteAsyncControl {
public:
 static constexpr uint32_t PrimaryLifetime=2500U, BackupLifetime=15000U;
 static constexpr uint32_t PrimaryGap=150U, BackupGap=LoRaAirtimeProfile::BaselineGap;
 static constexpr uint32_t PrimaryHeartbeat=100U, BackupHeartbeat=LoRaAirtimeProfile::BaselineGap;
 static constexpr unsigned PrimaryBudget=8U;
 RemoteAsyncControl(RemoteRadioOwner& owner,RemoteAsyncTx& tx,LoRaOwnerTiming timing={}):owner_(owner),tx_(tx),timing_(timing){}
 RemoteAsyncControl(const RemoteAsyncControl&)=delete;
 RemoteAsyncControl& operator=(const RemoteAsyncControl&)=delete;
 bool invalidate(uint32_t now){
  if(!owner_.worker())return false;
  // Keep backup cooldown across STOP, session changes and ambiguous writes.
  if(backup_.ticket||tx_.pending(PhyLink::Backup))markBackup(now);
  active_=false;confirmed_=false;stopId_=0;attempts_=0;primary_=Pending{};backup_=Pending{};
  primaryGate_.reset();backupGate_.reset();backupConfirmed_=false;
  return tx_.invalidateSession();
 }
 // Conservative transfer of a possibly outstanding backup admission from handshake.
 void deferBackup(uint32_t now){if(owner_.worker())markBackup(now);}
 bool backupRecentlyUsed(uint32_t now)const{
  return owner_.worker()&&(backup_.ticket||(backupSent_&&uint32_t(now-lastBackup_)<BackupGap));
 }
 bool beginSession(uint8_t epoch,uint32_t now){
  if(!owner_.worker())return false;
  if(!invalidate(now)||!epoch||!tx_.beginSession(epoch))return false;
  epoch_=epoch;active_=true;lastPrimary_=now-PrimaryHeartbeat;
  return true;
 }
 bool requestStop(const BoatCommand& c,uint32_t now){
  if(!owner_.worker()||!active_||c.type!=CMD_EMERGENCY_STOP||
     c.commandId==0||c.commandId>UINT16_MAX||c.activate)return false;
  if(stopId_==c.commandId)return true; // duplicate intent never extends lifetime
  if(backup_.ticket||tx_.pending(PhyLink::Backup))markBackup(now);
  if(!tx_.requestStop(uint16_t(c.commandId))){invalidate(now);return false;}
  stop_=c;stopId_=uint16_t(c.commandId);started_=now;attempts_=0;
  primary_=Pending{};backup_=Pending{};primaryAttempted_=false;
  confirmed_=false;backupConfirmed_=false;primaryGate_.request(stopId_);
  backupGate_.request(stopId_);return true;
 }
 // Must be called only AFTER normal HMAC, session, sequence, payload and
 // command ACK tracker checks. The timestamp is local captured RX time.
 bool observeStopAck(PhyLink link,uint16_t id,bool accepted,uint32_t captured,uint32_t now){
  if(!owner_.worker()||!active_||!stopId_||confirmed_||id!=stopId_||
     !remoteRxFresh(now,captured)||uint32_t(captured-started_)>=BackupLifetime||
     (link!=PhyLink::Primary&&link!=PhyLink::Backup))return false;
  if(!primaryGate_.acceptedAck(id,accepted))return false;
  primaryAckAt_=captured;
  if(link==PhyLink::Backup&&backupGate_.acceptedAck(id,accepted))backupAckAt_=captured;
  return true;
 }
 bool observeStopStatus(PhyLink link,bool stopped,uint32_t captured,uint32_t now){
  if(!owner_.worker()||!active_||!stopId_||confirmed_||!stopped||
     !remoteRxFresh(now,captured)||uint32_t(captured-started_)>=BackupLifetime)return false;
  if(link==PhyLink::Backup){
   if(!backupGate_.ackAccepted()||!after(captured,backupAckAt_))return false;
   backupConfirmed_=true;return true; // cannot clear primary interlock
  }
  if(link!=PhyLink::Primary||!primaryGate_.ackAccepted()||!after(captured,primaryAckAt_))return false;
  if(backup_.ticket||tx_.pending(PhyLink::Backup))markBackup(now);
  if(!tx_.cancelStop(stopId_)){invalidate(now);return false;}
  primary_=Pending{};backup_=Pending{};confirmed_=true;return true;
 }
 // The application must supply independently validated CLEAR_STOP evidence.
 // Rollover revokes pending heartbeat reservations; retain backup pacing.
 bool rearmAfterClear(bool authorized,uint32_t now){
  if(!owner_.worker()||!active_||!confirmed_||!authorized)return false;
  if(backup_.ticket||tx_.pending(PhyLink::Backup))markBackup(now);
  if(!tx_.rearm(true))return false;
  primary_=Pending{};backup_=Pending{};stopId_=0;confirmed_=false;backupConfirmed_=false;
  primaryGate_.reset();backupGate_.reset();lastPrimary_=now-PrimaryHeartbeat;return true;
 }
 // Mission-only authorization: the normal transport latch remains closed.
 // Caller has independently validated EXECUTED CLEAR and fresh released STATUS.
 bool confirmMissionClear(uint16_t transaction,uint16_t command,bool authorized){
  if(!owner_.worker()||!active_||!confirmed_||!authorized||!tx_.confirmMissionClear(transaction,command,true))return false;
  stopId_=0;confirmed_=false;backupConfirmed_=false;primaryGate_.reset();backupGate_.reset();
  return true; // outstanding heartbeat tickets and backup spacing unchanged
 }
 bool inhibitOrdinary(uint32_t now){
  if(!owner_.worker()||!active_)return false;
  if(backup_.ticket||tx_.pending(PhyLink::Backup))markBackup(now);
  const bool ok=tx_.inhibitSessionTraffic();
  primary_=Pending{};backup_=Pending{};stopId_=0;confirmed_=backupConfirmed_=false;
  primaryGate_.reset();backupGate_.reset();lastPrimary_=now-PrimaryHeartbeat;
  if(!ok)active_=false;
  return ok;
 }
 // Caller revokes the primary producer/ledger before using the returned
 // backup-only scope. The previously authorized transaction is mandatory.
 bool transferMissionToBackup(uint16_t transaction,uint32_t now,bool primaryUnavailable){
  if(!owner_.worker()||!active_||stopId_||!primaryUnavailable)return false;
  if(backup_.ticket||tx_.pending(PhyLink::Backup))markBackup(now);
  if(!tx_.transferMissionToBackup(transaction,true))return false;
  primary_=Pending{};backup_=Pending{};confirmed_=backupConfirmed_=false;
  primaryGate_.reset();backupGate_.reset();lastPrimary_=now-PrimaryHeartbeat;
  return true;
 }
 // Owner calls after routed completions and BEFORE regular heartbeat tick.
 // STOP intent always excludes this reservation. Same conservative backup
 // cooldown is used by all producers, including on ambiguous completion.
 bool serviceBackupMission(BackupMissionControl& ledger,const BackupMissionAuthority& a,
                           uint32_t now,uint16_t& sequence,uint16_t rekeyAt,const uint8_t key[32]){
  if(!owner_.worker()||!active_||!key||!ledger.maintain(now,a))return false;
  const bool available=!stopId_&&!backup_.ticket&&!tx_.pending(PhyLink::Backup)&&
      !tx_.linkFault(PhyLink::Backup)&&(!backupSent_||uint32_t(now-lastBackup_)>=timing_.urgentGap());
  MissionControlRequest q{};
  if(!ledger.due(now,a,available,q))return true;
  if(rekeyAt<2||sequence>=uint16_t(rekeyAt-1))return false;
  RadioFrame f{};radio_frameInit(f,RADIO_MSG_MISSION_CONTROL,1,2,uint16_t(sequence+1),q.transaction,now,epoch_);
  f.flags=RADIO_FLAG_BACKUP_LINK;
  const uint32_t ttl=ledger.lifetime(now);
  if(!ttl||!missionControlEncode(q,f.payload)||!radio_frameSeal(f,key))return false;
  ++sequence;uint32_t ticket=0;const auto result=tx_.submitBackupMission(f,now,ttl,now,ticket,true);
  if(result==RemoteAsyncTx::Submit::Busy)return true;
  if(result!=RemoteAsyncTx::Submit::Queued)return false;
  markBackup(now);
  return ledger.queued(ticket,tx_.generation(),sequence,epoch_,now,ttl);
 }
 bool observeBackupMissionCompletion(BackupMissionControl& ledger,const RemoteAsyncTx::Result& result,uint32_t now){
  if(!owner_.worker()||!active_||result.link!=PhyLink::Backup||result.kind!=RemoteAsyncTx::Kind::Mission||
     result.stop||result.messageType!=RADIO_MSG_MISSION_CONTROL||result.epoch!=epoch_)return false;
  if(!ledger.complete(result.ticket,result.generation,result.sequence,result.epoch,result.commandId,result.createdMs,result.lifetimeMs))return false;
  markBackup(now); // local evidence only; conservative shared pacing
  return true;
 }
 // A raw-available primary lane can retry STOP before recovery is qualified.
 // Never extend the original STOP deadline or reset the retry budget.
 bool primaryStopOutstanding(uint32_t now)const{
  return owner_.worker()&&active_&&stopId_&&!confirmed_&&uint32_t(now-started_)<PrimaryLifetime&&attempts_<PrimaryBudget;
 }
 bool hasStopIntent(uint16_t id)const{return owner_.worker()&&active_&&id&&id==stopId_;}
 // A busy noncritical primary slot must not indefinitely defer the critical
 // heartbeat. The application revokes it before the next scheduler pass.
 bool ordinaryBlockingHeartbeat(uint32_t now)const{
  return owner_.worker()&&active_&&!primary_.ticket&&tx_.pending(PhyLink::Primary)&&
   uint32_t(now-lastPrimary_)>=PrimaryHeartbeat;
 }
 bool ordinaryGap(uint32_t now)const{
  return owner_.worker()&&active_&&(!stopId_||confirmed_)&&!primary_.ticket&&
   !tx_.pending(PhyLink::Primary)&&uint32_t(now-lastPrimary_)<PrimaryHeartbeat;
 }
 bool backupConfirmed()const{return owner_.worker()&&active_&&backupConfirmed_;}
 bool confirmed()const{return owner_.worker()&&active_&&confirmed_;}
 unsigned primaryReservations()const{return owner_.worker()?attempts_:0;}
 // Routed local completion only; never a peer ACK. Called on owner task.
 bool observeCompletion(const RemoteAsyncTx::Result& result,uint32_t now){
  if(!owner_.worker()||!active_||result.epoch!=epoch_||
     (result.link!=PhyLink::Primary&&result.link!=PhyLink::Backup))return false;
  auto& p=result.link==PhyLink::Primary?primary_:backup_;
  if(!p.ticket||p.ticket!=result.ticket||
     result.kind!=(p.stop?RemoteAsyncTx::Kind::Stop:RemoteAsyncTx::Kind::Heartbeat))return false;
  p=Pending{};
  if(result.link==PhyLink::Primary)lastPrimary_=now;else markBackup(now);
  return true;
 }
 // Completion first, then STOP before heartbeat. No direct PHY calls.
 // Sequence is shared with the application's other authenticated senders.
 bool tick(uint32_t now,bool primaryReady,bool backupReady,uint8_t heartbeatFlags,
           uint16_t& sequence,uint16_t rekeyAt,const uint8_t key[32],bool externallyRouted=false){
  if(!owner_.worker()||!active_||!key)return false;
  // Standalone legacy host harnesses retain self-drain. Integrated owners
  // must route centrally and pass externallyRouted=true.
  if(!externallyRouted&&(!drain(PhyLink::Primary,now)||!drain(PhyLink::Backup,now)))return false;
  if(rekeyAt<2||sequence>=uint16_t(rekeyAt-1)){invalidate(now);return false;}
  primaryReady=primaryReady&&!tx_.linkFault(PhyLink::Primary);
  backupReady=backupReady&&!tx_.linkFault(PhyLink::Backup);
  if(tx_.linkFault(PhyLink::Primary)&&tx_.linkFault(PhyLink::Backup)){invalidate(now);return false;}
  const bool primaryStop=primaryStopOutstanding(now);
  const bool backupStop=stopId_&&!confirmed_&&!backupConfirmed_&&uint32_t(now-started_)<BackupLifetime;
  if(primaryReady&&!primary_.ticket&&!tx_.pending(PhyLink::Primary)){
   if(primaryStop){
    if(!primaryAttempted_||uint32_t(now-lastPrimary_)>=PrimaryGap){
     if(!offer(PhyLink::Primary,true,now,heartbeatFlags,sequence,rekeyAt,key))return false;
    }
   }else if(uint32_t(now-lastPrimary_)>=PrimaryHeartbeat){
    if(!offer(PhyLink::Primary,false,now,heartbeatFlags,sequence,rekeyAt,key))return false;
   }
  }
  if(backupReady&&!backup_.ticket&&!tx_.pending(PhyLink::Backup)&&(!backupSent_||uint32_t(now-lastBackup_)>=(backupStop?timing_.urgentGap():BackupHeartbeat))){
   if(!offer(PhyLink::Backup,backupStop,now,heartbeatFlags,sequence,rekeyAt,key))return false;
  }
  return true;
 }
private:
 struct Pending{uint32_t ticket=0;bool stop=false;};
 static bool after(uint32_t a,uint32_t b){const uint32_t d=a-b;return d!=0&&d<0x80000000U;}
 void markBackup(uint32_t now){backupSent_=true;lastBackup_=now;}
 bool drain(PhyLink link,uint32_t now){
  RemoteAsyncTx::Result result;
  return !tx_.take(link,now,result)||observeCompletion(result,now);
 }
 bool offer(PhyLink link,bool stop,uint32_t now,uint8_t flags,
            uint16_t& sequence,uint16_t rekeyAt,const uint8_t key[32]){
  if(sequence>=uint16_t(rekeyAt-1)){invalidate(now);return false;}
  RadioFrame f{};
  const uint16_t next=uint16_t(sequence+1);
  if(stop){
   BoatCommand c=stop_;c.sequence=next;c.createdAtMs=now;
   if(!radio_encodeCommand(c,1,2,epoch_,f,key)){invalidate(now);return false;}
  }else{
   radio_frameInit(f,RADIO_MSG_HEARTBEAT,1,2,next,0,now,epoch_);
   f.payload[0]=flags;f.payload[1]=1;
  }
  f.flags=link==PhyLink::Backup?RADIO_FLAG_BACKUP_LINK:0;
  if(!radio_frameSeal(f,key)){invalidate(now);return false;}
  // Consume a sequence for each sealed candidate, even if local offer fails.
  sequence=next;
  const uint32_t created=stop?started_:now;
  const uint32_t ttl=stop?(link==PhyLink::Primary?PrimaryLifetime:BackupLifetime):
    (link==PhyLink::Primary?PrimaryHeartbeat:BackupHeartbeat);
  uint32_t ticket=0;
  const auto result=tx_.submit(link,f,created,ttl,now,ticket);
  if(result==RemoteAsyncTx::Submit::Busy)return true;
  if(result==RemoteAsyncTx::Submit::Fault&&tx_.linkFault(link)){
   if(link==PhyLink::Backup)markBackup(now);
   return true; // isolate sticky queue fault to its radio
  }
  if(result!=RemoteAsyncTx::Submit::Queued){invalidate(now);return false;}
  auto& p=link==PhyLink::Primary?primary_:backup_;p.ticket=ticket;p.stop=stop;
  if(link==PhyLink::Primary){
   lastPrimary_=now;
   // A queue reservation is conservatively counted, NOT claimed PHY admission.
   if(stop){++attempts_;primaryAttempted_=true;}
  }else markBackup(now);
  return true;
 }
 RemoteRadioOwner& owner_;RemoteAsyncTx& tx_;const LoRaOwnerTiming timing_;
 flytec_r30h17::StopConfirmationGate primaryGate_,backupGate_;
 Pending primary_,backup_;
 BoatCommand stop_{};
 uint32_t started_=0,lastPrimary_=0,lastBackup_=0,primaryAckAt_=0,backupAckAt_=0;
 unsigned attempts_=0;uint16_t stopId_=0;uint8_t epoch_=0;
 bool active_=false,confirmed_=false,backupConfirmed_=false,backupSent_=false,primaryAttempted_=false;
};
}
