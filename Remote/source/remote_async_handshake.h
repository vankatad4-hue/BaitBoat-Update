#pragma once
#include "remote_async_tx.h"
#include "radio_handshake.h"
// Experimental Core 1 root-key handshake. Caller supplies provisioned root,
// nonce and HMAC provider. No session traffic before verified CONFIRM.
namespace flytec_rtos {
class RemoteAsyncHandshake {
public:
 RemoteAsyncHandshake(RemoteRadioOwner& owner,RemoteAsyncTx& tx,SessionHandshake& session):
  owner_(owner),tx_(tx),session_(session){}
 RemoteAsyncHandshake(const RemoteAsyncHandshake&)=delete;
 RemoteAsyncHandshake& operator=(const RemoteAsyncHandshake&)=delete;
 bool reset(uint32_t now,bool deferBackup){
  if(!owner_.worker())return false;
  if(backupTicket_||deferBackup)markBackup(now);
  active_=false;primaryTicket_=backupTicket_=0;session_.reset();
  return tx_.invalidateSession();
 }
 bool start(uint64_t nonce,uint32_t now,uint32_t window=SessionHandshake::PrimaryWindowMs){
  if(!owner_.worker()||active_||session_.established()||!nonce)return false;
  if(!tx_.beginHandshake()||!session_.startRemote(nonce,now,window))return false;
  active_=true;started_=now;primarySent_=false;return true;
 }
 bool active()const{return owner_.worker()&&active_;}
 bool backupRecentlyUsed(uint32_t now)const{
  return owner_.worker()&&(backupTicket_||(backupSent_&&uint32_t(now-lastBackup_)<3000U));
 }
 bool tick(uint32_t now,bool primaryReady,bool backupReady,const uint8_t root[32]){
  if(!owner_.worker()||!active_||!root||session_.timeout(now))return false;
  uint8_t nonzero=0;for(unsigned i=0;i<32;++i)nonzero|=root[i];
  if(!nonzero)return false;
  drain(PhyLink::Primary,primaryTicket_,now);
  drain(PhyLink::Backup,backupTicket_,now);
  SessionHandshakePacket packet{};
  if(!session_.remotePacket(packet))return false;
  if(primaryReady&&!tx_.linkFault(PhyLink::Primary)&&!primaryTicket_&&
     (!primarySent_||uint32_t(now-lastPrimary_)>=500U)){
   if(!offer(PhyLink::Primary,packet,root,now,primaryTicket_))return false;
  }
  if(backupReady&&!tx_.linkFault(PhyLink::Backup)&&!backupTicket_&&
     (!backupSent_||uint32_t(now-lastBackup_)>=3000U)){
   if(!offer(PhyLink::Backup,packet,root,now,backupTicket_))return false;
  }
  return true;
 }
 // Invalid input is ignored, not a reason to destroy a good transcript.
 // Only validated fresh root-authenticated CHALLENGE/CONFIRM reaches session.
 bool receive(PhyLink link,const RadioFrame& f,uint32_t captured,uint32_t now,
              const uint8_t root[32],SessionHmac hmac){
  if(!owner_.worker()||!active_||!root||!remoteRxFresh(now,captured)||
     uint32_t(captured-started_)>=session_.windowMs()||
     (link!=PhyLink::Primary&&link!=PhyLink::Backup)||
     !radioLinkFlagMatches(f.flags,link==PhyLink::Backup)||
     f.sourceId!=2||f.destinationId!=1||f.sequence||f.commandId||f.sessionEpoch||
     (f.type!=RADIO_MSG_SESSION_CHALLENGE&&f.type!=RADIO_MSG_SESSION_CONFIRM)||
     !radio_frameVerify(f,root))return false;
  if(!session_.remoteReceive((SessionHandshakeMessage)f.type,f.payload,root,hmac,now,true)){
   if(!session_.established()){SessionHandshakePacket pending{};if(!session_.remotePacket(pending))reset(now,true);}
   return false;
  }
  if(backupTicket_)markBackup(now);
  primaryTicket_=backupTicket_=0;primarySent_=false;
  // New phase cancels old HELLO/PROOF, including already dequeued work.
  if(session_.established()){
   active_=false;
   if(tx_.invalidateSession())return true;
  }else if(tx_.beginHandshake())return true;
  reset(now,true);return false;
 }
private:
 void markBackup(uint32_t now){backupSent_=true;lastBackup_=now;}
 void drain(PhyLink l,uint32_t& ticket,uint32_t now){
  RemoteAsyncTx::Result c;
  if(!tx_.take(l,now,c)||!ticket||ticket!=c.ticket)return;
  ticket=0;if(l==PhyLink::Backup)markBackup(now);
 }
 bool offer(PhyLink link,const SessionHandshakePacket& packet,const uint8_t root[32],uint32_t now,uint32_t& ticket){
  RadioFrame f;radio_frameInit(f,(RadioMessageType)packet.type,1,2,0,0,now,0);
  memcpy(f.payload,packet.payload,8);
  f.flags=link==PhyLink::Backup?RADIO_FLAG_BACKUP_LINK:0;
  if(!radio_frameSeal(f,root))return false;
  const auto result=tx_.submit(link,f,started_,session_.windowMs(),now,ticket);
  if(result==RemoteAsyncTx::Submit::Busy)return true;
  if(result==RemoteAsyncTx::Submit::Fault&&tx_.linkFault(link))return true;
  if(result!=RemoteAsyncTx::Submit::Queued)return false;
  if(link==PhyLink::Backup)markBackup(now);
  else{primarySent_=true;lastPrimary_=now;}
  return true;
 }
 RemoteRadioOwner& owner_;RemoteAsyncTx& tx_;SessionHandshake& session_;
 uint32_t started_=0,primaryTicket_=0,backupTicket_=0,lastPrimary_=0,lastBackup_=0;
 bool active_=false,primarySent_=false,backupSent_=false;
};
}
