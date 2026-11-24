#pragma once
#include "radio_handshake.h"
// Separate recovery transcript. Caller verifies root HMAC, peer IDs and link.
// HELLO alone never replaces the current session.
class BoatSessionRecovery {
public:
 static constexpr uint32_t quietMs=10000U;
 static bool eligible(bool established,bool primaryFresh,bool backupFresh,uint32_t lastRx,uint32_t now){
  return established&&!primaryFresh&&!backupFresh&&uint32_t(now-lastRx)>=quietMs;
 }
 void reset(){candidate_.reset();}
 bool expire(uint32_t now){if(!candidate_.timeout(now))return false;reset();return true;}
 bool receive(bool allowed,SessionHandshakeMessage type,const uint8_t payload[8],
              uint64_t nonce,const uint8_t root[32],SessionHmac mac,uint32_t now,
              SessionHandshake& current,SessionHandshakePacket& reply,bool& replaced){
  replaced=false;
  if(!allowed||!current.established()){reset();return false;}
  if(type!=SESSION_HELLO&&type!=SESSION_PROOF)return false;
  bool fresh=false;
  if(!candidate_.boatReceive(type,payload,nonce,root,mac,now,true,reply,fresh))return false;
  if(fresh){current.reset();current=candidate_;candidate_.reset();replaced=true;}
  return true;
 }
private:
 SessionHandshake candidate_;
};
