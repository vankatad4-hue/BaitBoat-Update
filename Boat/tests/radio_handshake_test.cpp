#include "../source/radio_handshake.h"
#include <openssl/hmac.h>
#include <cassert>
#include <iostream>
static bool sha256(const uint8_t key[32],const uint8_t *bytes,size_t n,uint8_t out[32]){
 unsigned len=0;return HMAC(EVP_sha256(),key,32,bytes,n,out,&len)!=nullptr&&len==32;
}
int main(){
 uint8_t root[32];memset(root,0xab,sizeof(root));
 SessionHandshake remote,boat;SessionHandshakePacket x{},y{},oldProof{},oldConfirm{};bool newSession=false;
 assert(remote.startRemote(100,0));assert(remote.remotePacket(x)&&x.type==SESSION_HELLO);
 assert(!boat.boatReceive(x.type,x.payload,200,root,sha256,1,false,y,newSession));
 assert(boat.boatReceive(x.type,x.payload,200,root,sha256,1,true,y,newSession)&&!newSession);
 assert(remote.remoteReceive(y.type,y.payload,root,sha256,2,true));
 assert(remote.remotePacket(x)&&x.type==SESSION_PROOF);oldProof=x;
 assert(boat.boatReceive(x.type,x.payload,0,root,sha256,3,true,y,newSession)&&newSession);
 oldConfirm=y;
 assert(boat.boatReceive(x.type,x.payload,0,root,sha256,4,true,y,newSession)&&!newSession);
 assert(remote.remoteReceive(y.type,y.payload,root,sha256,5,true)&&remote.established());
 uint8_t a[32],b[32];assert(remote.frameKey(a)&&boat.frameKey(b)&&!memcmp(a,b,32));
 remote.reset();boat.reset();assert(remote.startRemote(101,10));assert(remote.remotePacket(x));
 assert(boat.boatReceive(x.type,x.payload,201,root,sha256,11,true,y,newSession));
 assert(!boat.boatReceive(oldProof.type,oldProof.payload,0,root,sha256,12,true,x,newSession));
 assert(remote.remoteReceive(y.type,y.payload,root,sha256,13,true));
 assert(!remote.remoteReceive(oldConfirm.type,oldConfirm.payload,root,sha256,14,true));
 assert(remote.remotePacket(x));assert(boat.boatReceive(x.type,x.payload,0,root,sha256,15,true,y,newSession)&&newSession);
 assert(remote.remoteReceive(y.type,y.payload,root,sha256,16,true));
 assert(remote.frameKey(b)&&memcmp(a,b,32));
 remote.reset();assert(remote.startRemote(102,20));assert(remote.timeout(8020));assert(!remote.frameKey(b));
 for(auto v:b)assert(!v);
 std::cout<<"Mutual session handshake/replay and retry tests passed\n";
}
