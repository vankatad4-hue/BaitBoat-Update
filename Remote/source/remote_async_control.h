#pragma once
#include "remote_async_tx.h"
#include "radio_command_codec.h"
#include "r30h17_stop_confirmation_gate.h"
// R30H-124G: Core 1 application-side STOP/heartbeat scheduling.
// Use ONLY after authenticated session establishment. Caller owns key,
// session sequence counter, link readiness and authenticated RX dispatch.
// This module does not enable a physical worker or authorize movement.
namespace flytec_rtos {
class RemoteAsyncControl {
public:
 static constexpr uint32_t PrimaryLifetime=2500U, BackupLifetime=15000U;
 static constexpr uint32_t PrimaryGap=150U, BackupGap=3000U;
 static constexpr uint32_t PrimaryHeartbeat=100U, BackupHeartbeat=3000U;
 static constexpr unsigned PrimaryBudget=8U;
 RemoteAsyncControl(RemoteRadioOwner& owner,RemoteAsyncTx& tx):owner_(owner),tx_(tx){}
 RemoteAsyncControl(const RemoteAsyncControl&)=delete;
 RemoteAsyncControl& operator=(const RemoteAsyncControl&)=delete;
 bool invalidate(uint32_t now){
  if(!owner_.worker())return false;
  // Keep backup cooldown across STOP, session changes and ambiguous writes.
  if(backup_.ticket)markBackup(now);
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
  if(backup_.ticket)markBackup(now);
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
  if(backup_.ticket)markBackup(now);
  if(!tx_.cancelStop(stopId_)){invalidate(now);return false;}
  primary_=Pending{};backup_=Pending{};confirmed_=true;return true;
 }
 // The application must supply independently validated CLEAR_STOP evidence.
 // Rollover revokes pending heartbeat reservations; retain backup pacing.
 bool rearmAfterClear(bool authorized,uint32_t now){
  if(!owner_.worker()||!active_||!confirmed_||!authorized)return false;
  if(backup_.ticket)markBackup(now);
  if(!tx_.rearm(true))return false;
  primary_=Pending{};backup_=Pending{};stopId_=0;confirmed_=false;backupConfirmed_=false;
  primaryGate_.reset();backupGate_.reset();lastPrimary_=now-PrimaryHeartbeat;return true;
 }
 bool inhibitOrdinary(uint32_t now){
  if(!owner_.worker()||!active_)return false;
  if(backup_.ticket)markBackup(now);
  const bool ok=tx_.inhibitSessionTraffic();
  primary_=Pending{};backup_=Pending{};stopId_=0;confirmed_=backupConfirmed_=false;
  primaryGate_.reset();backupGate_.reset();lastPrimary_=now-PrimaryHeartbeat;
  if(!ok)active_=false;
  return ok;
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
  const bool primaryStop=stopId_&&!confirmed_&&uint32_t(now-started_)<PrimaryLifetime&&attempts_<PrimaryBudget;
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
  if(backupReady&&!backup_.ticket&&!tx_.pending(PhyLink::Backup)&&(!backupSent_||uint32_t(now-lastBackup_)>=BackupGap)){
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
 RemoteRadioOwner& owner_;RemoteAsyncTx& tx_;
 flytec_r30h17::StopConfirmationGate primaryGate_,backupGate_;
 Pending primary_,backup_;
 BoatCommand stop_{};
 uint32_t started_=0,lastPrimary_=0,lastBackup_=0,primaryAckAt_=0,backupAckAt_=0;
 unsigned attempts_=0;uint16_t stopId_=0;uint8_t epoch_=0;
 bool active_=false,confirmed_=false,backupConfirmed_=false,backupSent_=false,primaryAttempted_=false;
};
}
