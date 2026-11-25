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
 TxRequest last{};unsigned starts=0;bool ready=true;
 static bool send(void* c,const TxRequest& r){auto& d=*static_cast<Driver*>(c);if(!d.ready)return false;d.last=r;++d.starts;return true;}
};
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;RemoteAsyncTx tx(owner,h,gate);
 RemoteAuthenticatedMockApp app(owner,h,tx);RemoteMockPeer peer(mac);
 assert(app.initialize(peer.root(),mac));BoundedTxService service;Driver p,b;
 constexpr auto P=PhyLink::Primary,B=PhyLink::Backup;
 auto tick=[&](uint32_t n,bool fault=false){return app.tick(n,123,fault,false);};
 auto poll=[&](uint32_t n){assert(service.tick(h,P,gate,&Driver::send,&p,n));};
 auto deliver=[&](uint32_t n){RadioFrame f{};assert(peer.exchange(p.last.frame,f,false,n));assert(h.primaryRx.offer({f,n,P}));assert(h.offerCompletion(P,{p.last.ticket,p.last.generation,P,true,TxEvidence::NrfAck}));};
 auto status=[&](uint32_t n){RadioFrame f{};assert(peer.status(f,false,n));assert(h.primaryRx.offer({f,n,P}));};
 assert(tick(1));poll(1);deliver(2);assert(tick(2));poll(2);deliver(3);
 assert(tick(3)&&app.established());assert(app.requestStop(17,4));assert(tick(4));poll(4);
 // If stale heartbeat occupied the slot, it is discarded before the STOP.
 if(p.last.frame.commandId!=17)poll(4);
 assert(p.last.frame.commandId==17);deliver(5);assert(tick(5));status(6);assert(tick(6)&&app.confirmed());
 assert(!app.requestClear(18,6));app.mockInput(true,true);
 assert(app.requestClear(18,6));poll(6);
 assert(p.last.frame.payload[0]==CMD_CLEAR_STOP&&gate.stopped()&&!app.rearmed());
 deliver(7);assert(tick(7)&&!app.rearmed()&&gate.stopped());
 status(8);assert(tick(8)&&app.rearmed()&&!gate.stopped());
 poll(8);deliver(9);assert(tick(9));
 BoatCommand c{};c.type=CMD_RELEASE_HOPPER_LEFT;c.commandId=19;c.createdAtMs=10;c.activate=true;
 assert(app.requestOrdinary(c,10));assert(tick(10));poll(10);
 assert(p.last.frame.commandId==19);deliver(11);assert(tick(11));RadioCommandAckRecord ack{};
 assert(app.commandAck(19,ack)&&ack.state==RADIO_CMD_ACK_EXECUTED&&peer.mockReleases()==1);
 c.commandId=20;c.createdAtMs=12;assert(app.requestOrdinary(c,12));assert(tick(12));
 p.ready=false;poll(12);const auto before=p.starts;
 assert(tick(13,true)&&app.established()&&!app.rearmed()&&gate.stopped());
 p.ready=true;poll(14);
 assert(p.starts==before&&peer.mockReleases()==1);
 // Stale root request and canceled primary command drain without RF.
 assert(service.tick(h,B,gate,&Driver::send,&b,14)&&b.starts==0);
 assert(!app.requestStop(17,14)); // old intent cannot be reported as newly active
 assert(!app.requestOrdinary(c,14));assert(!app.requestClear(21,14));
 assert(app.requestStop(22,14));assert(tick(14,true));
 assert(tick(3014,true));assert(service.tick(h,B,gate,&Driver::send,&b,3014));
 assert(b.last.frame.commandId==22&&b.last.frame.flags==RADIO_FLAG_BACKUP_LINK);
 RadioFrame backupAck{};assert(peer.exchange(b.last.frame,backupAck,true,3015));
 assert(h.backupRx.offer({backupAck,3015,B}));
 assert(h.offerCompletion(B,{b.last.ticket,b.last.generation,B,false,TxEvidence::LocalReady}));
 assert(tick(3015,true)&&!app.confirmed());
 RadioFrame backupStatus{};assert(peer.status(backupStatus,true,3016));
 assert(h.backupRx.offer({backupStatus,3016,B}));assert(tick(3016,true));
 assert(app.backupConfirmed()&&!app.confirmed()&&!app.rearmed()&&gate.stopped());
 assert(!app.tick(3017,123,true,true)&&!app.established()&&gate.stopped());
 app.retire(3017);assert(peer.mockReleases()==1);
}
