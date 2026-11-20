#pragma once
#include "radio_session_v2.h"
// All input messages require an independently root-HMAC-verified envelope.
// HELLO/CHALLENGE carry 64-bit nonces; PROOF/CONFIRM carry role-separated proofs.
// The session frame key is never sent. No legacy-session fallback.
enum SessionHandshakeMessage:uint8_t{SESSION_HELLO=10,SESSION_CHALLENGE=11,SESSION_PROOF=12,SESSION_CONFIRM=13};
enum SessionHandshakePhase:uint8_t{HS_IDLE,HS_WAIT_CHALLENGE,HS_WAIT_PROOF,HS_WAIT_CONFIRM,HS_ESTABLISHED};
struct SessionHandshakePacket{SessionHandshakeMessage type;uint8_t payload[8];};
inline void sessionNonceWrite(uint64_t n,uint8_t out[8]){for(unsigned i=0;i<8;++i)out[i]=(uint8_t)(n>>(8*i));}
inline uint64_t sessionNonceRead(const uint8_t in[8]){uint64_t n=0;for(unsigned i=0;i<8;++i)n|=(uint64_t)in[i]<<(8*i);return n;}
class SessionHandshake{
public:
 void reset(){radioSession_wipe(&keys_,sizeof(keys_));transcript_={};phase_=HS_IDLE;started_=0;}
 bool startRemote(uint64_t nonce,uint32_t now){
  if(!nonce||phase_!=HS_IDLE)return false;
  transcript_={nonce,0,1,2};started_=now;phase_=HS_WAIT_CHALLENGE;return true;
 }
 bool timeout(uint32_t now)const{return phase_!=HS_IDLE&&phase_!=HS_ESTABLISHED&&now-started_>=8000U;}
 bool remotePacket(SessionHandshakePacket &out)const{
  if(phase_==HS_WAIT_CHALLENGE){out.type=SESSION_HELLO;sessionNonceWrite(transcript_.remoteNonce,out.payload);return true;}
  if(phase_==HS_WAIT_CONFIRM){out.type=SESSION_PROOF;memcpy(out.payload,keys_.remoteProof,8);return true;}
  return false;
 }
 bool remoteReceive(SessionHandshakeMessage type,const uint8_t payload[8],const uint8_t root[32],SessionHmac hmac,uint32_t now,bool valid){
  if(!valid||!payload||timeout(now))return false;
  if(type==SESSION_CHALLENGE&&phase_==HS_WAIT_CHALLENGE){
   transcript_.boatNonce=sessionNonceRead(payload);
   if(!radioSession_derive(root,transcript_,hmac,keys_)){reset();return false;}
   phase_=HS_WAIT_CONFIRM;return true;
  }
  if(type==SESSION_CONFIRM&&phase_==HS_WAIT_CONFIRM&&radioSession_verifyProof(keys_.boatProof,payload)){
   phase_=HS_ESTABLISHED;return true;
  }
  return false;
 }
 bool boatReceive(SessionHandshakeMessage type,const uint8_t payload[8],uint64_t freshNonce,const uint8_t root[32],
                  SessionHmac hmac,uint32_t now,bool valid,SessionHandshakePacket &out,bool &newSession){
  newSession=false;if(!valid||!payload)return false;
  if(timeout(now))reset();
  if(type==SESSION_HELLO){
   const uint64_t remote=sessionNonceRead(payload);
   if(!remote||phase_==HS_ESTABLISHED)return false;
   if(phase_==HS_WAIT_PROOF&&remote!=transcript_.remoteNonce)return false;
   if(phase_==HS_IDLE){
    transcript_={remote,freshNonce,1,2};
    if(!radioSession_derive(root,transcript_,hmac,keys_)){reset();return false;}
    started_=now;phase_=HS_WAIT_PROOF;
   }
   if(phase_!=HS_WAIT_PROOF)return false;
   out.type=SESSION_CHALLENGE;sessionNonceWrite(transcript_.boatNonce,out.payload);return true;
  }
  if(type==SESSION_PROOF&&(phase_==HS_WAIT_PROOF||phase_==HS_ESTABLISHED)&&radioSession_verifyProof(keys_.remoteProof,payload)){
   newSession=phase_==HS_WAIT_PROOF;phase_=HS_ESTABLISHED;
   out.type=SESSION_CONFIRM;memcpy(out.payload,keys_.boatProof,8);return true;
  }
  return false;
 }
 bool established()const{return phase_==HS_ESTABLISHED;}
 bool frameKey(uint8_t out[32])const{
  if(!out)return false;
  if(!established()){radioSession_wipe(out,32);return false;}
  memcpy(out,keys_.frameKey,32);return true;
 }
private:
 RadioSessionTranscript transcript_{};RadioSessionKeys keys_{};SessionHandshakePhase phase_=HS_IDLE;uint32_t started_=0;
};
