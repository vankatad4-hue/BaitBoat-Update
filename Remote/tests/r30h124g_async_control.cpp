#include "remote_async_control.h"
#include "bounded_tx_service.h"
#include <cassert>
using namespace flytec_rtos;
constexpr auto P=PhyLink::Primary,B=PhyLink::Backup;
struct Phy{
 TxRequest request{};unsigned starts=0;
 static bool send(void* p,const TxRequest& r){auto& d=*static_cast<Phy*>(p);d.request=r;++d.starts;return true;}
};
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;
 RemoteAsyncTx tx(owner,h,gate);RemoteAsyncControl app(owner,tx);
 BoundedTxService service;Phy p,b;
 uint8_t key[32]={1};uint16_t seq=0;
 auto tick=[&](uint32_t n,bool primary=true,bool backup=true){return app.tick(n,primary,backup,7,seq,65000,key);};
 auto poll=[&](PhyLink l,Phy& phy,uint32_t now){assert(service.tick(h,l,gate,&Phy::send,&phy,now));};
 auto complete=[&](PhyLink l,Phy& phy){const auto& r=phy.request;assert(h.offerCompletion(l,TxCompletion{r.ticket,r.generation,l,l==P,l==P?TxEvidence::NrfAck:TxEvidence::LocalReady}));};
 assert(app.beginSession(1,0));assert(tick(0));poll(P,p,0);poll(B,b,0);
 assert(radio_frameVerify(p.request.frame,key)&&radio_frameVerify(b.request.frame,key));
 assert(p.request.frame.type==RADIO_MSG_HEARTBEAT&&b.request.frame.flags==RADIO_FLAG_BACKUP_LINK);
 complete(P,p);complete(B,b);assert(tick(1));
 BoatCommand stop{};stop.type=CMD_EMERGENCY_STOP;stop.mode=MODE_MANUAL;stop.commandId=17;
 assert(app.requestStop(stop,2));assert(tick(2));poll(P,p,2);
 assert(p.request.frame.commandId==17&&radio_frameVerify(p.request.frame,key));
 assert(app.primaryReservations()==1);complete(P,p);assert(tick(3));
 assert(!app.confirmed()&&gate.stopped());
 assert(!app.observeStopStatus(P,true,4,4));
 assert(!app.observeStopAck(P,16,true,5,5));
 assert(app.observeStopAck(B,17,true,6,6));
 assert(!app.observeStopStatus(P,true,6,6));
 assert(app.observeStopStatus(B,true,7,7)&&!app.confirmed());
 assert(!app.observeStopStatus(P,true,7,107)); // stale
 assert(app.observeStopStatus(P,true,8,8)&&app.confirmed()&&gate.stopped());
 assert(app.requestStop(stop,9)&&app.confirmed()); // repeated intent not new
 assert(tick(103));poll(P,p,103);assert(p.request.frame.type==RADIO_MSG_HEARTBEAT);
 complete(P,p);assert(tick(104));
 assert(app.beginSession(1,105)&&!app.confirmed());
 stop.commandId=18;assert(app.requestStop(stop,106));
 assert(tick(106));poll(P,p,106);assert(p.request.frame.commandId==18);
 complete(P,p);assert(tick(107));
 assert(!app.observeStopAck(P,17,true,108,108));
 // Count bounded primary reservations; retry spacing begins on terminal result.
 for(unsigned i=1;i<8;i++){
  uint32_t n=107+i*151U;assert(tick(n,true,false));poll(P,p,n);
  assert(p.request.frame.commandId==18);complete(P,p);assert(tick(n+1,true,false));
 }
 assert(app.primaryReservations()==8);
 assert(tick(1400,true,false));poll(P,p,1400);assert(p.request.frame.type==RADIO_MSG_HEARTBEAT);
 complete(P,p);assert(tick(1401,true,false));
 // Backup pacing survives session reset; it cannot bypass prior local completion.
 assert(tick(3000,false,true));assert(b.starts==1);
 assert(tick(3001,false,true));poll(B,b,3001);
 assert(b.starts==2&&b.request.frame.commandId==18&&radio_frameVerify(b.request.frame,key));
 assert(app.invalidate(3002));assert(!tick(3003));assert(gate.stopped());
}
