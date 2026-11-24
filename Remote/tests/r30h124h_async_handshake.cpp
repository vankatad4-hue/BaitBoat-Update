#include "remote_async_handshake.h"
#include "remote_async_control.h"
#include "remote_worker_runtime.h"
#include "bounded_tx_service.h"
#include <mbedtls/md.h>
#include <cassert>
using namespace flytec_rtos;
constexpr auto P=PhyLink::Primary,B=PhyLink::Backup;
static bool mac(const uint8_t key[32],const uint8_t* data,size_t n,uint8_t out[32]){
 return mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),key,32,data,n,out)==0;
}
struct Driver {
 TxRequest last{};unsigned starts=0;bool ready=true;
 static bool send(void* ctx,const TxRequest& r){
  auto& d=*static_cast<Driver*>(ctx);if(!d.ready)return false;
  d.last=r;++d.starts;return true;
 }
};
struct Worker{
 bool done=false;unsigned stops=0;
 RadioWatchdog::State health=RadioWatchdog::State::Healthy;
 bool begin(bool a){return a;}
 void requestStop(){++stops;}
 bool exited()const{return done;}
 bool permit()const{return true;}
 RadioWatchdog::State supervise(){return health;}
};
struct Service{bool service(uint32_t){return true;}};
struct Retirement{
 RemoteRadioOwner& owner;RemoteAsyncHandshake& handshake;RemoteAsyncControl& control;unsigned calls=0;
 static void run(void* p){auto& r=*static_cast<Retirement*>(p);++r.calls;
  r.control.invalidate(100);r.handshake.reset(100,true);r.owner.retire();}
};
static RadioFrame response(const SessionHandshakePacket& p,const uint8_t root[32],PhyLink link){
 RadioFrame f;radio_frameInit(f,(RadioMessageType)p.type,2,1,0,0,0,0);
 memcpy(f.payload,p.payload,8);f.flags=link==B?RADIO_FLAG_BACKUP_LINK:0;
 assert(radio_frameSeal(f,root));return f;
}
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;RemoteAsyncTx tx(owner,h,gate);
 SessionHandshake remote,boat;RemoteAsyncHandshake hs(owner,tx,remote);RemoteAsyncControl control(owner,tx);
 Worker w;Service svc;RemoteWorkerRuntime<Worker,Service> runtime(w,svc);
 Retirement retire{owner,hs,control};assert(runtime.bindRetirement(&Retirement::run,&retire));
 assert(runtime.begin(true,true));assert(runtime.tick(0));
 uint8_t root[32]={42},key[32]={},peerKey[32]={};
 BoundedTxService pump;Driver p,b;
 auto poll=[&](PhyLink l,Driver& d,uint32_t n){assert(pump.tick(h,l,gate,&Driver::send,&d,n));};
 assert(hs.start(123,0));assert(!tx.rearm(true));
 RadioFrame f{};f.type=RADIO_MSG_HEARTBEAT;uint32_t ticket=0;
 assert(tx.submit(P,f,0,100,0,ticket)==RemoteAsyncTx::Submit::Denied);
 assert(hs.tick(0,true,true,root));poll(P,p,0);
 assert(p.last.frame.type==RADIO_MSG_SESSION_HELLO&&radio_frameVerify(p.last.frame,root));
 const auto hello=p.last;
 b.ready=false;poll(B,b,0);assert(b.starts==0);
 SessionHandshakePacket packet{};bool newSession=false;
 assert(boat.boatReceive(SESSION_HELLO,p.last.frame.payload,456,root,mac,1,true,packet,newSession));
 auto challenge=response(packet,root,P);
 auto corrupt=challenge;corrupt.authTag[0]^=1;
 assert(!hs.receive(P,corrupt,1,1,root,mac));
 assert(!hs.receive(B,challenge,1,1,root,mac));
 assert(!hs.receive(P,challenge,1,101,root,mac));
 assert(hs.receive(P,challenge,2,2,root,mac));
 b.ready=true;poll(B,b,2);assert(b.starts==0); // old dequeued HELLO revoked
 assert(h.offerCompletion(P,TxCompletion{hello.ticket,hello.generation,P,true,TxEvidence::NrfAck}));
 assert(hs.tick(2,true,true,root));poll(P,p,2);
 assert(p.last.frame.type==RADIO_MSG_SESSION_PROOF&&radio_frameVerify(p.last.frame,root));
 assert(boat.boatReceive(SESSION_PROOF,p.last.frame.payload,456,root,mac,3,true,packet,newSession)&&newSession);
 auto confirm=response(packet,root,P);
 assert(hs.receive(P,confirm,3,3,root,mac)&&remote.established());
 assert(remote.frameKey(key)&&boat.frameKey(peerKey)&&memcmp(key,peerKey,32)==0);
 assert(control.beginSession(1,3));
 if(hs.backupRecentlyUsed(3))control.deferBackup(3);
 uint16_t seq=0;assert(control.tick(3,true,true,7,seq,30000,key));poll(P,p,3);
 assert(p.last.frame.type==RADIO_MSG_HEARTBEAT&&radio_frameVerify(p.last.frame,key)&&!radio_frameVerify(p.last.frame,root));
 poll(B,b,3);assert(b.starts==0); // pacing crosses session transition
 // Rekey cancels pending authenticated TX and resets sequence authority.
 seq=29999;assert(!control.tick(4,true,true,7,seq,30000,key));assert(gate.stopped());
 assert(hs.reset(4,control.backupRecentlyUsed(4)));
 assert(hs.start(789,4));assert(hs.tick(4,true,false,root));poll(P,p,4);
 assert(p.last.frame.type==RADIO_MSG_SESSION_HELLO);
 assert(!hs.receive(P,confirm,5,5,root,mac)); // old transcript CONFIRM
 assert(!hs.tick(8004,true,false,root));assert(hs.reset(8004,true));
 assert(hs.start(987,8005));assert(hs.tick(8005,true,false,root));poll(P,p,8005);
 boat.reset();
 assert(boat.boatReceive(SESSION_HELLO,p.last.frame.payload,654,root,mac,8006,true,packet,newSession));
 assert(hs.receive(P,response(packet,root,P),8006,8006,root,mac));
 assert(hs.tick(8006,true,false,root));poll(P,p,8006);
 assert(boat.boatReceive(SESSION_PROOF,p.last.frame.payload,654,root,mac,8007,true,packet,newSession));
 assert(hs.receive(P,response(packet,root,P),8007,8007,root,mac)&&remote.established());
 uint8_t nextKey[32]={};assert(remote.frameKey(nextKey)&&memcmp(key,nextKey,32)!=0);
 assert(control.beginSession(1,8007));seq=0;
 assert(control.tick(8007,true,false,7,seq,30000,nextKey));poll(P,p,8007);
 assert(p.last.frame.sequence==1&&radio_frameVerify(p.last.frame,nextKey));
 assert(!radio_frameVerify(p.last.frame,key));
 w.health=RadioWatchdog::State::Fault;assert(!runtime.tick(8008));
 assert(retire.calls==1&&w.stops==1&&!owner.worker()&&!owner.claimLegacy());
 runtime.requestStop();assert(retire.calls==1&&w.stops==1&&!runtime.exited());
 w.done=true;assert(runtime.exited()&&!runtime.begin(true,true));
}
