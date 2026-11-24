#pragma once
#include "remote_async_handshake.h"
#include "remote_async_control.h"
#include "radio_ack_tracker.h"
// R30H-124J: authenticated application composition for isolated mock runtime.
// Deliberately does NOT own physical drivers or authorize ordinary/mission TX.
namespace flytec_rtos {
class RemoteAuthenticatedMockApp {
public:
 RemoteAuthenticatedMockApp(RemoteRadioOwner& owner,RadioFrameHandoff& handoff,RemoteAsyncTx& tx):
  owner_(owner),handoff_(handoff),tx_(tx),handshake_(owner,tx,session_),control_(owner,tx){}
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
  if(id==stopId_)return true;
  // Only STOP is admitted by this application, including when one lane failed.
  BoatCommand c{};c.type=CMD_EMERGENCY_STOP;c.mode=MODE_MANUAL;c.commandId=id;
  if(!control_.requestStop(c,now))return false;
  if(!ack_.begin(id,CMD_EMERGENCY_STOP,MODE_MANUAL,now,15000U)){reset(now);return false;}
  stopId_=id;return true;
 }
 bool tick(uint32_t now,uint64_t nonce,bool primaryFault,bool backupFault){
  if(!owner_.worker()||!initialized_)return false;
  primaryFault_|=primaryFault||tx_.linkFault(PhyLink::Primary);
  backupFault_|=backupFault||tx_.linkFault(PhyLink::Backup);
  if(primaryFault_&&backupFault_)return false;
  if((session_.established()&&sequence_>=RadioSessionCounter::REKEY_AT-1)||session_.timeout(now)){
   if(!reset(now))return false;
  }
  consume(PhyLink::Primary,now);consume(PhyLink::Backup,now);
  if(!initialized_)return false;
  if(!session_.established()){
   if(!handshake_.active()&&!handshake_.start(nonce,now,SessionHandshake::windowFor(!backupFault_)))return false;
   return handshake_.tick(now,!primaryFault_,!backupFault_,root_);
  }
  ack_.update(now);
  const uint8_t flags=uint8_t(1U|(!primaryFault_?2U:0U)|(!backupFault_?4U:0U)|(control_.confirmed()?8U:0U));
  if(!control_.tick(now,!primaryFault_,!backupFault_,flags,sequence_,RadioSessionCounter::REKEY_AT,key_))
   return reset(now);
  return true;
 }
 void retire(uint32_t now){
  if(!owner_.worker())return;
  reset(now);radioSession_wipe(root_,32);initialized_=false;mac_=nullptr;
 }
private:
 bool reset(uint32_t now){
  const bool defer=control_.backupRecentlyUsed(now);
  const bool a=control_.invalidate(now);
  const bool b=handshake_.reset(now,defer);
  radioSession_wipe(key_,32);sequence_=stopId_=0;ack_.reset(0);
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
   RadioDecodedPayload decoded{};
   if(f.type==RADIO_MSG_ACK){
    if(!radio_decodeAckPayload(f.payload,decoded)||f.commandId!=decoded.commandId||decoded.commandType!=CMD_EMERGENCY_STOP)continue;
   }else if(f.type==RADIO_MSG_TELEMETRY){
    if(f.commandId||!radio_decodeTelemetryPayload(f.payload,decoded)||decoded.kind!=RADIO_PAYLOAD_STATUS)continue;
   }else if(f.type==RADIO_MSG_HEARTBEAT){
    if(f.commandId||!radio_decodeHeartbeatPayload(f.payload,decoded))continue;
   }else continue;
   auto& guard=link==PhyLink::Primary?primary_:backup_;
   if(!guard.accept(f.sequence)||!cross_.accept(f.sequence))continue;
   if(decoded.kind==RADIO_PAYLOAD_ACK&&ack_.observe(decoded,link==PhyLink::Primary?RADIO_ACK_LINK_PRIMARY:RADIO_ACK_LINK_BACKUP,1,rx.receivedMs)){
    control_.observeStopAck(link,decoded.commandId,decoded.ackResult==RADIO_ACK_ACCEPTED||decoded.ackResult==RADIO_ACK_EXECUTED,rx.receivedMs,now);
   }else if(decoded.kind==RADIO_PAYLOAD_STATUS){
    control_.observeStopStatus(link,decoded.stopState==1,rx.receivedMs,now);
   }
  }
 }
 RemoteRadioOwner& owner_;RadioFrameHandoff& handoff_;RemoteAsyncTx& tx_;
 SessionHandshake session_;RemoteAsyncHandshake handshake_;RemoteAsyncControl control_;
 RadioAckTracker ack_;RadioSessionCounter primary_,backup_,cross_;
 uint8_t root_[32]{},key_[32]{};SessionHmac mac_=nullptr;
 uint16_t sequence_=0,stopId_=0;
 bool initialized_=false,primaryFault_=false,backupFault_=false;
};
}
