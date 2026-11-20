#include "../source/radio_session_v2.h"
#include <openssl/hmac.h>
#include <cassert>
#include <iostream>
static bool sha256(const uint8_t key[32],const uint8_t *bytes,size_t n,uint8_t out[32]){
 unsigned len=0;return HMAC(EVP_sha256(),key,32,bytes,n,out,&len)!=nullptr&&len==32;
}
int main(){
 uint8_t root[32];memset(root,0xAB,sizeof(root));
 RadioSessionTranscript t={0x1122334455667788ULL,0xA0B1C2D3E4F50617ULL,1,2};
 RadioSessionKeys a{},b{};
 assert(radioSession_derive(root,t,sha256,a)&&radioSession_derive(root,t,sha256,b));
 assert(!memcmp(a.frameKey,b.frameKey,32));
 assert(radioSession_verifyProof(a.remoteProof,b.remoteProof));
 assert(!radioSession_verifyProof(a.remoteProof,a.boatProof));
 ++t.boatNonce;assert(radioSession_derive(root,t,sha256,b));
 assert(memcmp(a.frameKey,b.frameKey,32)&&!radioSession_verifyProof(a.remoteProof,b.remoteProof));
 t.boatNonce=0;assert(!radioSession_derive(root,t,sha256,b));
 for(auto v:b.frameKey)assert(v==0);
 t.boatNonce=7;memset(root,0,sizeof(root));assert(!radioSession_derive(root,t,sha256,b));
 RadioSessionCounter tx,rx;uint16_t seq=0;
 assert(tx.next(seq)&&seq==1&&rx.accept(seq));assert(!rx.accept(seq)&&!rx.accept(0));
 assert(rx.accept(500)&&!rx.accept(499)&&!rx.accept(30000));
 rx.resetAfterHandshake();assert(rx.accept(1));
 for(unsigned i=2;i<30000;++i)assert(tx.next(seq)&&seq==i);
 assert(!tx.next(seq));
 std::cout<<"Session V2 transcript, proof separation and replay bounds passed\n";
}
