#include "remote_async_tx.h"
#include "bounded_tx_service.h"
#include <cassert>
#include <thread>
using namespace flytec_rtos;
using A=RemoteAsyncTx;
struct Driver {
 unsigned starts=0;TxRequest last{};bool ready=true;
 static bool start(void* p,const TxRequest& r){
  auto& d=*static_cast<Driver*>(p);if(!d.ready)return false;
  ++d.starts;d.last=r;return true;
 }
};
static RadioFrame frame(PhyLink l,uint16_t id,bool stop){
 RadioFrame f{};f.type=RADIO_MSG_ACTION;f.sessionEpoch=1;f.commandId=id;
 f.flags=l==PhyLink::Backup?RADIO_FLAG_BACKUP_LINK:0;
 f.payload[0]=stop?CMD_EMERGENCY_STOP:CMD_SET_MODE;return f;
}
int main(){
 assert(bindRemoteI2cOwner());RemoteRadioOwner owner;assert(owner.claimWorker(true));
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate gate;
 A tx(owner,h,gate);BoundedTxService service;Driver d;A::Result out;uint32_t t=0,old=0;
 constexpr auto p=PhyLink::Primary,b=PhyLink::Backup;
 assert(tx.beginSession(1));assert(tx.submit(p,frame(p,1,false),0,100,0,t)==A::Submit::Denied);
 assert(tx.rearm(true));
 assert(tx.submit(p,frame(p,1,false),0,100,0,old)==A::Submit::Queued);
 d.ready=false;assert(service.tick(h,p,gate,&Driver::start,&d,1));assert(d.starts==0);
 assert(tx.requestStop(9));assert(tx.submit(p,frame(p,8,true),2,100,2,t)==A::Submit::Denied);
 assert(tx.submit(p,frame(p,9,true),2,100,2,t)==A::Submit::Queued);
 const auto stopTicket=t;assert(tx.submit(b,frame(b,9,true),2,100,2,t)==A::Submit::Queued);
 d.ready=true;assert(service.tick(h,p,gate,&Driver::start,&d,3));assert(d.last.ticket==stopTicket);
 assert(h.offerCompletion(p,TxCompletion{old,d.last.generation,p,true,TxEvidence::NrfAck}));
 assert(h.offerCompletion(p,TxCompletion{stopTicket,d.last.generation,b,true,TxEvidence::NrfAck}));
 assert(!tx.take(p,3,out));
 assert(h.offerCompletion(p,TxCompletion{stopTicket,d.last.generation,p,true,TxEvidence::NrfAck}));
 assert(tx.take(p,4,out)&&out.stop&&out.commandId==9&&out.outcome==A::Outcome::NrfAck);
 assert(gate.stopped()&&!tx.take(p,4,out));
 assert(service.tick(h,b,gate,&Driver::start,&d,4));
 assert(h.offerCompletion(b,TxCompletion{d.last.ticket,d.last.generation,b,false,TxEvidence::LocalReady}));
 assert(tx.take(b,5,out)&&out.stop&&out.commandId==9&&out.outcome==A::Outcome::LocalReady);
 // Original deadline survives dequeue and busy retry, including wrap.
 assert(tx.rearm(true));const auto start=UINT32_MAX-4;
 assert(tx.submit(p,frame(p,2,false),start,10,start,t)==A::Submit::Queued);
 const auto before=d.starts;d.ready=false;
 assert(service.tick(h,p,gate,&Driver::start,&d,1));
 d.ready=true;assert(service.tick(h,p,gate,&Driver::start,&d,5));
 assert(d.starts==before&&tx.take(p,5,out)&&out.outcome==A::Outcome::Expired);
 assert(tx.submit(p,frame(p,3,false),5,10,15,t)==A::Submit::Expired);
 assert(tx.submit(p,frame(p,3,false),5,100,5,t)==A::Submit::Queued);
 assert(service.tick(h,p,gate,&Driver::start,&d,6));const auto prior=d.last;
 assert(tx.beginSession(1)&&tx.rearm(true));
 assert(tx.submit(p,frame(p,3,false),7,100,7,t)==A::Submit::Queued);
 assert(h.offerCompletion(p,TxCompletion{prior.ticket,prior.generation,p,true,TxEvidence::NrfAck}));
 assert(!tx.take(p,7,out));assert(service.tick(h,p,gate,&Driver::start,&d,8));
 assert(d.last.ticket==t&&d.last.generation!=prior.generation);
 std::thread foreign([&]{uint32_t v=0;assert(tx.submit(b,frame(b,4,false),8,100,8,v)==A::Submit::Denied);assert(!tx.invalidateSession());});
 foreign.join();assert(tx.invalidateSession());
 assert(tx.submit(p,frame(p,3,false),8,100,8,t)==A::Submit::Denied);
}
