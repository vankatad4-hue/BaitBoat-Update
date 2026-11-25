#pragma once
#include "remote_async_handshake.h"
#include "remote_tx_router.h"
#include "remote_clear_gate.h"
#include "remote_async_ordinary.h"
#include "remote_async_control.h"
#include "radio_ack_tracker.h"
#include "mission_wire.h"
#include "remote_mission_prepare_tx.h"
// R30H-124J: authenticated application composition for isolated mock runtime.
// Isolated mock only. No physical driver, real joystick, or mission authority.
namespace flytec_rtos {
class RemoteAuthenticatedMockApp {
public:
 RemoteAuthenticatedMockApp(RemoteRadioOwner& owner,RadioFrameHandoff& handoff,RemoteAsyncTx& tx):
  owner_(owner),handoff_(handoff),tx_(tx),handshake_(owner,tx,session_),control_(owner,tx),ordinary_(tx),mission_(tx){}
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
 bool requestStop(uint16_t id,uint32_t now){
  if(!established()||!id)return false;
  if(id==stopId_)return control_.hasStopIntent(id)&&!rearmed_&&!clear_.pending()&&!missionLocked();
  // STOP is always prioritized, including when one lane failed.
  BoatCommand c{};c.type=CMD_EMERGENCY_STOP;c.mode=MODE_MANUAL;c.commandId=id;
  if(!control_.requestStop(c,now))return false;
  clear_.reset();clearTicket_=0;ordinary_.intent().transportRevoked();rearmed_=false;
  if(missionLocked())mission_.upload().transportRevoked();
  if(!ack_.begin(id,CMD_EMERGENCY_STOP,MODE_MANUAL,now,15000U)){reset(now);return false;}
  stopId_=id;return true;
 }
 // Explicit simulated input, never a substitute for real calibrated sticks.
 void mockInput(bool valid,bool neutral){if(owner_.worker()){inputValid_=valid;inputNeutral_=neutral;}}
 bool rearmed()const{return established()&&rearmed_;}
 bool commandAck(uint16_t id,RadioCommandAckRecord& out)const{return owner_.worker()&&ack_.get(id,out);}
 bool requestClear(uint16_t id,uint32_t now){
  RadioCommandAckRecord old{};
  if(!established()||!id||rearmed_||missionLocked()||!control_.confirmed()||!control_.ordinaryGap(now)||
     !primaryStatusSeen_||primaryStatus_.stopState!=1||ack_.get(id,old)||sequence_>=RadioSessionCounter::REKEY_AT-1)return false;
  const auto auth=clearAuth(now);
  if(!clear_.begin(id,1,now,true,auth))return false;
  BoatCommand cmd{};cmd.type=CMD_CLEAR_STOP;cmd.mode=MODE_MANUAL;cmd.commandId=id;cmd.sequence=++sequence_;cmd.createdAtMs=now;
  RadioFrame f{};
  if(!radio_encodeCommand(cmd,1,2,1,f,key_)){clear_.reset();return false;}
  const auto result=tx_.submitClear(f,now,100U,now,clearTicket_,true);
  if(result!=RemoteAsyncTx::Submit::Queued){clear_.reset();return false;}
  if(!ack_.begin(id,CMD_CLEAR_STOP,MODE_MANUAL,now,RemoteClearGate::Deadline)){inhibit(now);return false;}
  return true;
 }
 bool requestOrdinary(const BoatCommand& cmd,uint32_t now){
  if(!rearmed()||clear_.pending()||missionLocked())return false;
  RadioCommandAckRecord old{};
  if(cmd.type!=CMD_MANUAL_MOTORS&&ack_.get(uint16_t(cmd.commandId),old))return false;
  return ordinary_.intent().stage(cmd,ordinaryAuth(now),now);
 }
 // Mock-only preparation. Battery percentage here is simulated telemetry;
 // real production must also check fresh measured battery voltage.
 bool requestMission(const MissionPlan& plan,uint32_t now){
  return owner_.worker()&&missionSafe(now)&&mission_.upload().begin(plan,1,now,true);
 }
 bool missionPrepared(uint32_t now)const{return owner_.worker()&&missionSafe(now)&&mission_.upload().prepared(now);}
 bool missionLocked()const{return mission_.upload().state()!=AsyncMissionUpload::State::Idle;}
 // Local cancellation only. Peer may have retained a preparation: keep
 // CLEAR and movement blocked until a fresh session or future handoff.
 bool cancelMissionLocal(uint32_t now){return owner_.worker()&&missionLocked()&&inhibit(now);}
 bool tick(uint32_t now,uint64_t nonce,bool primaryFault,bool backupFault){
  if(!owner_.worker()||!initialized_)return false;
  primaryFault_|=primaryFault||tx_.linkFault(PhyLink::Primary);
  backupFault_|=backupFault||tx_.linkFault(PhyLink::Backup);
  if(primaryFault_&&backupFault_){retire(now);return false;}
  if((session_.established()&&sequence_>=RadioSessionCounter::REKEY_AT-1)||session_.timeout(now)){
   if(!reset(now))return false;
  }
  consume(PhyLink::Primary,now);consume(PhyLink::Backup,now);
  if(!initialized_)return false;
  // Fail closed before draining results when authority was withdrawn.
  if(missionActive()&&!mission_.upload().maintain(now,missionSafe(now))){if(!inhibit(now))return reset(now);}
  if(mission_.upload().state()==AsyncMissionUpload::State::Failed&&mission_.upload().inFlight()){if(!inhibit(now))return reset(now);}
  if(clear_.pending()||clear_.state()==RemoteClearGate::State::Failed){
   if(!clear_.maintain(now,clearAuth(now))&&!inhibit(now))return reset(now);
  }
  if(rearmed_&&!ordinaryAuth(now).actuatorReady){if(!inhibit(now))return reset(now);}
  if(ordinary_.intent().state()==OrdinaryIntent::State::InFlight&&!ordinary_.intent().stillAllowed(ordinaryAuth(now),now)){
   if(!inhibit(now))return reset(now);
  }
  if(!routeRemoteTx(tx_,now,
      [&](const RemoteAsyncTx::Result& c,uint32_t at){return handshake_.observeCompletion(c,at);},
      [&](const RemoteAsyncTx::Result& c,uint32_t at){return control_.observeCompletion(c,at);},
      [&](const RemoteAsyncTx::Result& c,uint32_t at){
       if(clearTicket_&&c.ticket==clearTicket_&&c.commandId==clear_.id()&&c.link==PhyLink::Primary&&c.epoch==1&&c.messageType==RADIO_MSG_ACTION){clearTicket_=0;return true;}
       return ordinary_.observeCompletion(c,at);
      },[&](const RemoteAsyncTx::Result& c,uint32_t){return mission_.observeCompletion(c);}))return reset(now);
  if(!session_.established()){
   if(!handshake_.active()&&!handshake_.start(nonce,now,SessionHandshake::windowFor(!backupFault_)))return false;
   return handshake_.tick(now,!primaryFault_,!backupFault_,root_,true);
  }
  ack_.update(now);
  // Do not allow a busy ordinary/CLEAR slot to delay primary heartbeat.
  // The shared generation revocation keeps the STOP latch closed and
  // preserves backup cooldown; already admitted RF is not retractable.
  if(control_.ordinaryBlockingHeartbeat(now)){
   if(!inhibit(now))return reset(now);
  }
  if(clear_.ready(now,clearAuth(now))&&primaryStatus_.stopState==0&&(primaryStatus_.safetyFlags&1U)){
   if(!control_.rearmAfterClear(true,now))return reset(now);
   clear_.reset();clearTicket_=0;rearmed_=true;
  }
  const uint8_t flags=uint8_t(1U|(!primaryFault_?2U:0U)|(!backupFault_?4U:0U)|(control_.confirmed()?8U:0U));
  if(!control_.tick(now,!primaryFault_,!backupFault_,flags,sequence_,RadioSessionCounter::REKEY_AT,key_,true))
   return reset(now);
  if(missionActive()&&!mission_.service(now,missionSafe(now),control_.ordinaryGap(now),1,sequence_,RadioSessionCounter::REKEY_AT,key_)){
   if(!inhibit(now))return reset(now);
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
 bool missionActive()const{
  const auto s=mission_.upload().state();return s==AsyncMissionUpload::State::Active||s==AsyncMissionUpload::State::Prepared;
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
  if(ok){ordinary_.intent().transportRevoked();if(missionLocked())mission_.upload().transportRevoked();}
  clear_.reset();clearTicket_=0;rearmed_=false;
  return ok;
 }
 bool reset(uint32_t now){
  const bool defer=control_.backupRecentlyUsed(now);
  const bool a=control_.invalidate(now);
  const bool b=handshake_.reset(now,defer);
  radioSession_wipe(key_,32);sequence_=stopId_=0;ack_.reset(0);
  clear_.reset();clearTicket_=0;ordinary_.intent().reset();rearmed_=false;primaryStatusSeen_=false;inputValid_=inputNeutral_=false;
  if(a&&b)mission_.upload().resetAfterSession();
  cross_.resetAfterHandshake();primary_.resetAfterHandshake();backup_.resetAfterHandshake();
  return a&&b;
 }
 void consume(PhyLink link,uint32_t now){
  auto& queue=link==PhyLink::Primary?handoff_.primaryRx:handoff_.backupRx;
  RxFrame rx{};
  for(unsigned i=0;i<4&&queue.take(rx);++i){
   if(rx.link!=link||(link==PhyLink::Primary?primaryFault_:backupFault_)||!remoteRxFresh(now,rx.receivedMs))continue;
   const RadioFrame& f=rx.frame;
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
    }
   }
  }
 }
 RemoteRadioOwner& owner_;RadioFrameHandoff& handoff_;RemoteAsyncTx& tx_;
 SessionHandshake session_;RemoteAsyncHandshake handshake_;RemoteAsyncControl control_;
 RemoteClearGate clear_;RemoteAsyncOrdinary<RemoteAsyncTx> ordinary_;RemoteMissionPrepareTx<RemoteAsyncTx> mission_;
 RadioDecodedPayload primaryStatus_{};uint32_t primaryStatusAt_=0,clearTicket_=0;
 bool primaryStatusSeen_=false,inputValid_=false,inputNeutral_=false,rearmed_=false;
 RadioAckTracker ack_;RadioSessionCounter primary_,backup_,cross_;
 uint8_t root_[32]{},key_[32]{};SessionHmac mac_=nullptr;
 uint16_t sequence_=0,stopId_=0;
 bool initialized_=false,primaryFault_=false,backupFault_=false;
};
}
