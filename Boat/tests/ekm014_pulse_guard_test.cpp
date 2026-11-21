#include <cassert>
#include "../source/ekm014_pulse_guard.h"
int main(){
 Ekm014PulseGuard g;uint16_t w[4]={100,100,100,100};
 Ekm014Feedback f{true,true,true,true,100};
 assert(!g.configure(w,false));assert(!g.request(0,100,f));
 assert(g.configure(w,true));assert(g.request(0,100,f));
 assert(!g.request(1,101,f));f.sampledAt=149;assert(g.tick(149,f)==0);
 f.sampledAt=201;f.allCoilCurrentsZero=false;
 assert(g.tick(201,f)==-1);assert(g.awaitingZero());
 assert(!g.request(1,201,f));
 f.sampledAt=202;f.allCoilCurrentsZero=true;
 assert(g.tick(202,f)==-1);assert(!g.awaitingZero());
 assert(g.request(2,202,f));
 assert(g.tick(260,f)==-1); // stale feedback locks out
 assert(!g.request(3,260,f));
}
