#include "boat_stop_delivery.h"
#include <cassert>
using namespace boat_r30h18;
int main(){
 static_assert(StopAckOutbox::MAX_AGE_MS==2500U,"primary");
 static_assert(BackupStopAckOutbox::MAX_AGE_MS==12000U,"backup");
 BackupStopAckOutbox b;
 const uint32_t t=0xffffff00U;
 assert(b.offer(1,2,0,1,0,t));
 assert(!b.due(t+3000U,true,true,true));
 assert(b.pending());
 assert(b.offer(1,2,1,1,0,t+6000U));
 assert(b.ticket().result==1);
 assert(b.due(t+11999U,true,true,false));
 assert(!b.due(t+12000U,true,true,false));
 assert(!b.pending());
 assert(b.offer(2,2,0,1,0,t));
 assert(!b.due(t+1U,false,true,false));
 assert(!b.pending());
 assert(b.offer(2,3,0,1,0,t));
 assert(!b.due(t+1U,true,false,false));
 assert(!b.pending());
 assert(b.offer(3,3,0,1,0,t));
 b.reset();
 b.completed(true);
 assert(!b.pending());
 StopAckOutbox p;
 assert(p.offer(1,2,0,1,0,t));
 assert(p.due(t+2499U,true,true,false));
 assert(!p.due(t+2500U,true,true,false));
}
