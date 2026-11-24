#include "boat_session_recovery.h"
#include <mbedtls/md.h>
#include <cassert>
static bool mac(const uint8_t k[32],const uint8_t* d,size_t n,uint8_t out[32]){
 return mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),k,32,d,n,out)==0;
}
int main(){
 uint8_t root[32]={42},oldKey[32]{},newKey[32]{},peerKey[32]{};
 SessionHandshake current,oldRemote,newRemote;
 SessionHandshakePacket request{},reply{};bool changed=false;
 assert(oldRemote.startRemote(111,0)&&oldRemote.remotePacket(request));
 assert(current.boatReceive(request.type,request.payload,222,root,mac,0,true,reply,changed)&&!changed);
 assert(oldRemote.remoteReceive(reply.type,reply.payload,root,mac,1,true)&&oldRemote.remotePacket(request));
 assert(current.boatReceive(request.type,request.payload,222,root,mac,2,true,reply,changed)&&changed);
 assert(oldRemote.remoteReceive(reply.type,reply.payload,root,mac,3,true)&&current.frameKey(oldKey));
 const auto oldProof=request;
 BoatSessionRecovery recovery;
 assert(!BoatSessionRecovery::eligible(true,false,false,3,10002));
 assert(BoatSessionRecovery::eligible(true,false,false,3,10003));
 assert(!BoatSessionRecovery::eligible(true,false,true,3,10003));
 assert(BoatSessionRecovery::eligible(true,false,false,UINT32_MAX-4,9995));
 assert(newRemote.startRemote(333,10003)&&newRemote.remotePacket(request));
 assert(!recovery.receive(false,request.type,request.payload,444,root,mac,10003,current,reply,changed));
 assert(recovery.receive(true,request.type,request.payload,444,root,mac,10003,current,reply,changed)&&!changed);
 assert(current.frameKey(newKey)&&memcmp(oldKey,newKey,32)==0);
 assert(!recovery.receive(true,oldProof.type,oldProof.payload,444,root,mac,10004,current,reply,changed));
 assert(newRemote.remoteReceive(SESSION_CHALLENGE,reply.payload,root,mac,10004,true)&&newRemote.remotePacket(request));
 recovery.reset();
 assert(!recovery.receive(true,request.type,request.payload,444,root,mac,10005,current,reply,changed));
 newRemote.reset();assert(newRemote.startRemote(555,10006)&&newRemote.remotePacket(request));
 assert(recovery.receive(true,request.type,request.payload,666,root,mac,10006,current,reply,changed)&&!changed);
 assert(newRemote.remoteReceive(reply.type,reply.payload,root,mac,10007,true)&&newRemote.remotePacket(request));
 assert(recovery.receive(true,request.type,request.payload,666,root,mac,10008,current,reply,changed)&&changed);
 assert(newRemote.remoteReceive(reply.type,reply.payload,root,mac,10009,true));
 assert(current.frameKey(newKey)&&newRemote.frameKey(peerKey)&&memcmp(newKey,peerKey,32)==0);
 assert(memcmp(oldKey,newKey,32)!=0);
 assert(!recovery.receive(true,oldProof.type,oldProof.payload,777,root,mac,10010,current,reply,changed));
}
