#include "remote_primary_recovery_gate.h"
#include <cassert>
using flytec_rtos::PrimaryRecoveryGate;
int main(){
 PrimaryRecoveryGate g;assert(!g.begin(0,1)&&g.begin(1,7));
 assert(g.offer(10,100,1,7)&&!g.offer(11,101,1,7));
 assert(!g.observe(10,101,101,1,7,false));
 assert(!g.observe(9,101,101,1,7,true));
 assert(!g.observe(10,101,101,1,8,true));
 assert(!g.observe(10,99,101,1,7,true));
 assert(g.observe(10,101,101,1,7,true));
 assert(!g.observe(10,102,102,1,7,true));
 assert(!g.offer(10,102,1,7));
 assert(g.offer(11,600,1,7)&&g.observe(11,601,601,1,7,true));
 assert(!g.ready(601,1,7));
 assert(g.offer(12,1100,1,7)&&g.observe(12,1101,1101,1,7,true));
 assert(g.ready(1101,1,7)&&!g.ready(1101,1,8)&&!g.ready(1851,1,7));
 assert(g.offer(13,1852,1,7)&&g.observe(13,1853,1853,1,7,true)&&!g.ready(1853,1,7));
 assert(g.offer(14,2000,1,7));assert(!g.observe(14,2250,2250,1,7,true));
 assert(g.offer(15,2251,1,7)&&g.observe(15,2252,2252,1,7,true)&&!g.ready(2252,1,7));
 assert(g.begin(2,8)&&!g.observe(15,2253,2253,1,7,true));
 uint32_t t=0xfffffe00U;assert(g.offer(1,t,2,8)&&g.observe(1,t+1,t+1,2,8,true));
 t+=500;assert(g.offer(2,t,2,8)&&g.observe(2,t+1,t+1,2,8,true));
 t+=500;assert(g.offer(3,t,2,8)&&g.observe(3,t+1,t+1,2,8,true)&&g.ready(t+1,2,8));
 g.reset();assert(!g.ready(t+1,2,8));
}
