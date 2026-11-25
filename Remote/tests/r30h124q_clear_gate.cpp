#include "remote_clear_gate.h"
#include <cassert>
using namespace flytec_rtos;
using S=RemoteClearGate::State;
int main(){
 RemoteClearGate g;ClearAuthorization a{};
 assert(!g.begin(1,1,10,true,a));a={true,true,true,true,true};
 assert(!g.begin(1,1,10,false,a));assert(g.begin(1,1,10,true,a));
 assert(!g.begin(2,1,11,true,a));assert(!g.status(1,true,true,true,11,11));
 assert(!g.ack(1,1,true,false,false,11,11)); // ACCEPTED is not EXECUTED
 assert(!g.ack(1,1,false,true,false,12,12));assert(!g.ack(2,1,true,true,false,12,12));
 assert(!g.ack(1,2,true,true,false,12,12));assert(!g.ack(1,1,true,true,false,12,112));
 assert(g.ack(1,1,true,true,false,13,13));assert(!g.ready(13,a));
 assert(!g.status(1,true,true,true,13,13));assert(!g.status(1,false,true,true,14,14));
 assert(!g.status(1,true,false,true,14,14));assert(!g.status(1,true,true,false,14,14));
 assert(g.status(1,true,true,true,15,15));assert(g.ready(15,a)&&!g.ready(115,a));
 a.neutral=false;assert(!g.ready(16,a));assert(!g.maintain(16,a)&&g.state()==S::Failed);
 g.reset();a.neutral=true;assert(g.begin(2,1,100,true,a));assert(g.ack(2,1,true,false,true,101,101));
 assert(g.state()==S::Failed&&!g.ready(102,a)&&!g.begin(3,1,102,true,a));
 g.reset();assert(g.begin(3,1,100,true,a));assert(g.maintain(1599,a)&&!g.maintain(1600,a));
 g.reset();assert(g.begin(4,1,UINT32_MAX-5,true,a));assert(g.ack(4,1,true,true,false,0,0));
 assert(g.status(1,true,true,true,1,1)&&g.ready(1,a));
}
