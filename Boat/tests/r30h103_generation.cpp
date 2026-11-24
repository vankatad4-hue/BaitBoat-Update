#include "tx_generation_gate.h"
#include <cassert>
#include <thread>
#include <atomic>
using G=flytec_rtos::TxGenerationGate;
static bool accept(void* p){++*static_cast<unsigned*>(p);return true;}
struct Barrier{std::atomic<bool> entered{false},release{false};};
static bool blocked(void* p){auto& b=*static_cast<Barrier*>(p);b.entered.store(true);while(!b.release.load())std::this_thread::yield();return true;}
int main(){
 {G g;Barrier b;assert(g.transition(1));
  std::thread phy([&]{assert(g.tryAdmit(1,blocked,&b)==G::Result::Accepted);});
  while(!b.entered.load())std::this_thread::yield();
  assert(!g.latchStop(2)&&g.stopped());
  assert(!g.rearm(3,true));
  b.release.store(true);phy.join();
  unsigned n=0;assert(g.tryAdmit(1,accept,&n)==G::Result::Busy&&n==0);
  assert(g.latchStop(2));
  assert(g.tryAdmit(2,accept,&n)==G::Result::Inhibited&&n==0);
  assert(g.tryAdmit(2,accept,&n,true)==G::Result::Accepted&&n==1);
  assert(g.rearm(3,true));assert(g.tryAdmit(2,accept,&n)==G::Result::Stale);
 }
 G g;unsigned n=0;
 assert(g.tryAdmit(0,accept,&n)==G::Result::Stale);
 assert(g.transition(1));assert(g.tryAdmit(1,accept,&n)==G::Result::Accepted);
 assert(g.transition(2));assert(!g.transition(1));
 assert(g.tryAdmit(1,accept,&n)==G::Result::Stale&&n==1);
 assert(g.tryAdmit(2,accept,&n)==G::Result::Accepted&&n==2);
 assert(g.transition(UINT32_MAX));assert(!g.transition(0));
}
