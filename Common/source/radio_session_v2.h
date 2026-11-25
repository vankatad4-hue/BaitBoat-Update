#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
// Session V2 candidate. Portable transcript/key derivation and monotonic
// per-direction replay guard. Not connected to the legacy live radio adapter.
// Both peers must supply fresh nonces from a hardware CSPRNG and authenticate
// the challenge/response under their physically provisioned unique secret.
typedef bool (*SessionHmac)(const uint8_t key[32],const uint8_t *data,size_t n,uint8_t out[32]);
struct RadioSessionTranscript{uint64_t remoteNonce,boatNonce;uint8_t remoteId,boatId;};
struct RadioSessionKeys{uint8_t frameKey[32],remoteProof[8],boatProof[8];};
inline void radioSession_wipe(void *ptr,size_t n){
 volatile uint8_t *p=static_cast<volatile uint8_t*>(ptr);while(n--)*p++=0;
}
inline bool radioSession_derive(const uint8_t root[32],const RadioSessionTranscript &t,SessionHmac hmac,RadioSessionKeys &out){
 radioSession_wipe(&out,sizeof(out));
 if(!root||!hmac||!t.remoteNonce||!t.boatNonce||t.remoteNonce==t.boatNonce||!t.remoteId||!t.boatId||t.remoteId==t.boatId)return false;
 uint8_t any=0;for(unsigned i=0;i<32;++i)any|=root[i];if(!any)return false;
 // Fixed-width canonical transcript: protocol domain, peer roles, both nonces.
 uint8_t msg[27]={'F','L','Y','T','E','C','2',0,t.remoteId,t.boatId,0};
 for(unsigned i=0;i<8;++i){msg[11+i]=(uint8_t)(t.remoteNonce>>(8*i));msg[19+i]=(uint8_t)(t.boatNonce>>(8*i));}
 uint8_t digest[32]={};
 for(uint8_t role=1;role<=3;++role){
  msg[10]=role;
  if(!hmac(root,msg,sizeof(msg),digest)){radioSession_wipe(digest,sizeof(digest));radioSession_wipe(&out,sizeof(out));return false;}
  if(role==1)memcpy(out.remoteProof,digest,8);
  else if(role==2)memcpy(out.boatProof,digest,8);
  else memcpy(out.frameKey,digest,32);
 }
 radioSession_wipe(digest,sizeof(digest));return true;
}
inline bool radioSession_verifyProof(const uint8_t expected[8],const uint8_t supplied[8]){
 if(!expected||!supplied)return false;
 uint8_t difference=0;for(unsigned i=0;i<8;++i)difference|=expected[i]^supplied[i];return difference==0;
}
// No wrapping or reset by incoming packets. Reset only when a fresh mutually
// confirmed transcript establishes a new frame key. Cross-radio retries need
// fresh sequence numbers, plus stable logical action IDs for deduplication.
class RadioSessionCounter{
public:
 static constexpr uint16_t REKEY_AT=30000;
 bool accept(uint16_t counter){
  if(!counter||counter>=REKEY_AT||counter<=last_)return false;
  last_=counter;return true;
 }
 bool next(uint16_t &counter){
  if(last_>=REKEY_AT-1)return false;
  counter=++last_;return true;
 }
 void resetAfterHandshake(){last_=0;}
private:uint16_t last_=0;
};
