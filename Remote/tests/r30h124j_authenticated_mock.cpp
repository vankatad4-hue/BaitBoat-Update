#include "remote_authenticated_mock_app.h"
#include "remote_mock_peer.h"
#include "bounded_tx_service.h"
#include <mbedtls/md.h>
#include <cassert>
using namespace flytec_rtos;
static bool mac(const uint8_t k[32],const uint8_t* d,size_t n,uint8_t out[32]){
 return mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),k,32,d,n,out)==0;
}
struct Driver{
 TxRequest last{};unsigned starts=0;
 static bool send(void* c,const TxRequest& r){auto& d=*static_cast<Driver*>(c);d.last=r;++d.starts;return true;}
};
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;RemoteAsyncTx tx(owner,h,gate);
 RemoteAuthenticatedMockApp app(owner,h,tx);RemoteMockPeer peer(mac);
 assert(app.initialize(peer.root(),mac));assert(!app.requestStop(17,0));
 BoundedTxService service;Driver p,b;
 constexpr auto P=PhyLink::Primary,B=PhyLink::Backup;
 auto poll=[&](PhyLink l,Driver& d,uint32_t now){assert(service.tick(h,l,gate,&Driver::send,&d,now));};
 auto deliver=[&](PhyLink l,Driver& d,uint32_t now){
  RadioFrame f{};assert(peer.exchange(d.last.frame,f,l==B,now));
  auto& q=l==P?h.primaryRx:h.backupRx;assert(q.offer(RxFrame{f,now,l}));
  assert(h.offerCompletion(l,TxCompletion{d.last.ticket,d.last.generation,l,l==P,l==P?TxEvidence::NrfAck:TxEvidence::LocalReady}));
 };
 assert(app.tick(1,123,false,false));poll(P,p,1);deliver(P,p,2);
 assert(app.tick(2,123,false,false));poll(P,p,2);deliver(P,p,3);
 assert(app.tick(3,123,false,false)&&app.established());poll(P,p,3);
 assert(app.requestStop(17,4));assert(app.tick(4,123,false,false));poll(P,p,4);
 assert(p.last.frame.commandId==17);deliver(P,p,5);
 assert(app.tick(5,123,false,false)&&!app.confirmed());
 RadioFrame status{};assert(peer.status(status,false,6));assert(h.primaryRx.offer(RxFrame{status,6,P}));
 assert(app.tick(6,123,false,false)&&app.confirmed()&&gate.stopped());
 assert(app.tick(7,123,true,false));assert(app.requestStop(18,7));
 // Previous backup HELLO was queued but never admitted; drain stale generation.
 poll(B,b,8);assert(b.starts==0);
 assert(app.tick(3010,123,true,false));poll(B,b,3010);
 assert(b.last.frame.commandId==18);deliver(B,b,3011);
 assert(app.tick(3011,123,true,false)&&!app.confirmed());
 assert(peer.status(status,true,3012));assert(h.backupRx.offer(RxFrame{status,3012,B}));
 assert(app.tick(3012,123,true,false)&&app.backupConfirmed()&&!app.confirmed());
 assert(!app.tick(3013,123,true,true));app.retire(3013);owner.retire();
 assert(!app.established()&&!app.requestStop(19,3014)&&gate.stopped());
}
