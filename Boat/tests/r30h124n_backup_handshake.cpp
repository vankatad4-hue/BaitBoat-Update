#include "boat_session_recovery.h"
#include "boat_handshake_reply.h"
#include "boat_radio_safety.h"
#include <mbedtls/md.h>
#include <cassert>
static bool mac(const uint8_t k[32],const uint8_t* d,size_t n,uint8_t out[32]){
 return mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),k,32,d,n,out)==0;
}
static RadioFrame replyFrame(const SessionHandshakePacket& p){
 RadioFrame f{};f.type=uint8_t(p.type);f.sourceId=2;f.destinationId=1;f.flags=RADIO_FLAG_BACKUP_LINK;
 memcpy(f.payload,p.payload,8);return f;
}
int main(){
 uint8_t root[32]={42},oldKey[32]{},key[32]{},remoteKey[32]{};
 SessionHandshake boat,oldRemote,remote;
 SessionHandshakePacket request{},reply{};bool changed=false;
 assert(oldRemote.startRemote(111,0)&&oldRemote.remotePacket(request));
 assert(boat.boatReceive(request.type,request.payload,222,root,mac,0,true,reply,changed));
 assert(oldRemote.remoteReceive(reply.type,reply.payload,root,mac,1,true)&&oldRemote.remotePacket(request));
 assert(boat.boatReceive(request.type,request.payload,222,root,mac,2,true,reply,changed)&&changed);
 assert(oldRemote.remoteReceive(reply.type,reply.payload,root,mac,3,true)&&boat.frameKey(oldKey));
 const auto oldProof=request;
 const uint32_t lastRx=1000,start=1000;
 assert(remote.startRemote(333,start,SessionHandshake::windowFor(true)));
 assert(remote.windowMs()==20000U&&remote.remotePacket(request));
 BoatSessionRecovery recovery;BoatHandshakeReply backup;
 assert(!BoatSessionRecovery::eligible(true,false,false,lastRx,10999));
 assert(boatRadioLossRthDue(true,true,true,true,true,lastRx,11000));
 assert(recovery.receive(BoatSessionRecovery::eligible(true,false,false,lastRx,13000),
   request.type,request.payload,444,root,mac,13000,boat,reply,changed,SessionHandshake::BackupWindowMs)&&!changed);
 assert(backup.offer(replyFrame(reply),13000,true));
 assert(!backup.service(13000,6000,250,false,false,[](const RadioFrame&){return true;}));
 assert(!backup.service(13250,6000,250,true,true,[](const RadioFrame&){return true;}));
 assert(boat.frameKey(key)&&memcmp(key,oldKey,32)==0);
 assert(!recovery.receive(true,oldProof.type,oldProof.payload,444,root,mac,13500,boat,reply,changed,SessionHandshake::BackupWindowMs));
 assert(backup.service(16000,6000,250,true,false,[&](const RadioFrame& f){
  return remote.remoteReceive((SessionHandshakeMessage)f.type,f.payload,root,mac,16000,true);
 }));
 assert(remote.remotePacket(request)&&request.type==SESSION_PROOF);
 // Next remote transmission respects a 3000 ms gap. The original RTH clock stays due.
 assert(recovery.receive(true,request.type,request.payload,444,root,mac,19000,boat,reply,changed,SessionHandshake::BackupWindowMs)&&changed);
 assert(boatRadioLossRthDue(true,true,true,true,true,lastRx,19000));
 assert(backup.offer(replyFrame(reply),19000,true));
 assert(!backup.service(19000,6000,250,true,false,[](const RadioFrame&){return false;}));
 assert(backup.service(20000,6000,250,true,false,[&](const RadioFrame& f){
  return remote.remoteReceive((SessionHandshakeMessage)f.type,f.payload,root,mac,20000,true);
 }));
 assert(boat.frameKey(key)&&remote.frameKey(remoteKey)&&memcmp(key,remoteKey,32)==0&&memcmp(key,oldKey,32)!=0);
 SessionHandshake primary,expired;
 assert(primary.startRemote(555,1000)&&!primary.timeout(8999)&&primary.timeout(9000));
 assert(expired.startRemote(666,UINT32_MAX-10,SessionHandshake::BackupWindowMs));
 assert(!expired.timeout(19988)&&expired.timeout(19989));
 assert(!expired.startRemote(777,19989,SessionHandshake::BackupWindowMs));
}
