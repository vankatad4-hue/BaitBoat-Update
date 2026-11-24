#include "bounded_tx_service.h"
#include "bounded_rx_service.h"
#include <cassert>
using namespace flytec_rtos;
struct Fake{unsigned calls=0;uint32_t last=0;bool busy=true;};
static bool start(void* p,const TxRequest& r){auto& f=*static_cast<Fake*>(p);++f.calls;f.last=r.ticket;return !f.busy;}
static bool receive(void* p,RadioFrame& f){f.sequence=++*static_cast<unsigned*>(p);return true;}
int main(){
 {RadioFrameHandoff h;assert(h.begin());BoundedRxService rx;unsigned n=0;
  for(int i=0;i<4;++i)assert(rx.tick(h,PhyLink::Primary,123,receive,&n));
  assert(n==8);assert(!rx.tick(h,PhyLink::Primary,124,receive,&n)&&n==9&&h.fault());
  assert(!h.fault(PhyLink::Backup));assert(rx.tick(h,PhyLink::Backup,125,receive,&n)&&n==11);
  RxFrame f{};assert(h.primaryRx.take(f)&&f.frame.sequence==1&&f.receivedMs==123);
 }
 RadioFrameHandoff h;assert(h.begin());BoundedTxService s;Fake f;TxGenerationGate gate;assert(gate.transition(2));
 TxRequest r{};r.link=PhyLink::Primary;r.generation=1;r.ticket=1;
 assert(h.primaryCritical.offer(r));
 assert(s.tick(h,r.link,gate,start,&f)&&f.calls==0);
 TxCompletion c{};assert(h.takeCompletion(PhyLink::Primary,c)&&!c.success&&c.ticket==1);
 r.generation=2;r.ticket=2;assert(h.primaryCritical.offer(r));
 assert(s.tick(h,r.link,gate,start,&f)&&f.calls==1);
 f.busy=false;assert(s.tick(h,r.link,gate,start,&f)&&f.calls==2);
 assert(s.tick(h,r.link,gate,start,&f)&&f.calls==2);
 assert(!h.takeCompletion(PhyLink::Primary,c));
 r.ticket=3;assert(h.primaryTx.offer(r));f.busy=true;
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==3);
 r.ticket=4;assert(h.primaryCritical.offer(r));f.busy=false;
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==4);
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==3);
 // STOP invalidates queued old-generation traffic and inhibits current ordinary traffic.
 assert(gate.latchStop(3));
 r.generation=2;r.ticket=5;assert(h.primaryTx.offer(r));
 assert(s.tick(h,r.link,gate,start,&f));assert(h.takeCompletion(PhyLink::Primary,c)&&c.ticket==5&&!c.success);
 r.generation=3;r.ticket=6;assert(h.primaryTx.offer(r));
 assert(s.tick(h,r.link,gate,start,&f));assert(h.takeCompletion(PhyLink::Primary,c)&&c.ticket==6&&!c.success);
 r.ticket=7;assert(h.primaryCritical.offer(r));
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==7);
 assert(!gate.rearm(4,false)&&gate.stopped());
 assert(gate.rearm(4,true)&&!gate.stopped());
 r.generation=4;r.ticket=8;assert(h.primaryTx.offer(r));
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==8);
 const auto before=f.calls;
 for(int i=0;i<8;++i)assert(h.offerCompletion(PhyLink::Primary,c));
 r.generation=1;assert(h.primaryTx.offer(r));
 assert(!s.tick(h,r.link,gate,start,&f)&&h.fault());
 assert(!s.tick(h,r.link,gate,start,&f)&&f.calls==before);
 assert(h.fault(PhyLink::Primary)&&!h.fault(PhyLink::Backup));
 assert(gate.latchStop(5));r.link=PhyLink::Backup;r.generation=5;r.ticket=9;
 assert(h.backupCritical.offer(r));assert(s.tick(h,r.link,gate,start,&f)&&f.last==9);
 TxCompletion backup{9,5,PhyLink::Backup,true};assert(h.offerCompletion(r.link,backup));
 assert(h.takeCompletion(r.link,c)&&c.ticket==9&&c.success);
 // Reverse direction: backup overflow must not quarantine primary.
 {RadioFrameHandoff k;assert(k.begin());BoundedTxService b;TxGenerationGate g;assert(g.transition(1));
  for(int i=0;i<8;++i)assert(k.offerCompletion(PhyLink::Backup,backup));
  assert(!k.offerCompletion(PhyLink::Backup,backup)&&k.fault(PhyLink::Backup));
  r.link=PhyLink::Primary;r.generation=1;r.ticket=10;assert(k.primaryCritical.offer(r));
  assert(b.tick(k,r.link,g,start,&f)&&f.last==10&&!k.fault(r.link));
 }
}
