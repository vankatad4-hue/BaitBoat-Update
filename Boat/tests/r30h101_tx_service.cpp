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
  RxFrame f{};assert(h.primaryRx.take(f)&&f.frame.sequence==1&&f.receivedMs==123);
 }
 RadioFrameHandoff h;assert(h.begin());BoundedTxService s;Fake f;TxGenerationGate gate;assert(gate.transition(2));
 TxRequest r{};r.link=PhyLink::Primary;r.generation=1;r.ticket=1;
 assert(h.primaryCritical.offer(r));
 assert(s.tick(h,r.link,gate,start,&f)&&f.calls==0);
 TxCompletion c{};assert(h.completions.take(c)&&!c.success&&c.ticket==1);
 r.generation=2;r.ticket=2;assert(h.primaryCritical.offer(r));
 assert(s.tick(h,r.link,gate,start,&f)&&f.calls==1);
 f.busy=false;assert(s.tick(h,r.link,gate,start,&f)&&f.calls==2);
 assert(s.tick(h,r.link,gate,start,&f)&&f.calls==2);
 assert(!h.completions.take(c));
 r.ticket=3;assert(h.primaryTx.offer(r));f.busy=true;
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==3);
 r.ticket=4;assert(h.primaryCritical.offer(r));f.busy=false;
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==4);
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==3);
 // STOP invalidates queued old-generation traffic and inhibits current ordinary traffic.
 assert(gate.latchStop(3));
 r.generation=2;r.ticket=5;assert(h.primaryTx.offer(r));
 assert(s.tick(h,r.link,gate,start,&f));assert(h.completions.take(c)&&c.ticket==5&&!c.success);
 r.generation=3;r.ticket=6;assert(h.primaryTx.offer(r));
 assert(s.tick(h,r.link,gate,start,&f));assert(h.completions.take(c)&&c.ticket==6&&!c.success);
 r.ticket=7;assert(h.primaryCritical.offer(r));
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==7);
 assert(!gate.rearm(4,false)&&gate.stopped());
 assert(gate.rearm(4,true)&&!gate.stopped());
 r.generation=4;r.ticket=8;assert(h.primaryTx.offer(r));
 assert(s.tick(h,r.link,gate,start,&f)&&f.last==8);
 const auto before=f.calls;
 for(int i=0;i<8;++i)assert(h.completions.offer(c));
 r.generation=1;assert(h.primaryTx.offer(r));
 assert(!s.tick(h,r.link,gate,start,&f)&&h.fault());
 assert(!s.tick(h,r.link,gate,start,&f)&&f.calls==before);
}
