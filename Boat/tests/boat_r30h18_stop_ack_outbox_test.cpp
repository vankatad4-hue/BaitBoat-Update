#include "../source/boat_stop_delivery.h"
#include <cassert>
#include <iostream>
using boat_r30h18::StopAckOutbox;
int main(){
 StopAckOutbox q;
 assert(!q.pending()&&!q.due(0,true,true,false));
 assert(!q.offer(0,1,1,0,0,0));
 assert(!q.offer(3,0,1,0,0,0));
 assert(q.offer(42,3,1,0,0,1000));
 assert(!q.due(1010,true,true,true)); // previous TX still owns slot
 assert(q.pending()&&q.attempts()==0);
 assert(q.due(1010,true,true,false));q.admitted(1010);
 assert(q.inFlight()&&q.attempts()==1);
 assert(!q.due(1011,true,true,false));
 q.completed(false);assert(q.pending()&&!q.inFlight());
 assert(!q.due(1084,true,true,false));
 assert(q.due(1085,true,true,false));q.admitted(1085);
 q.completed(true);assert(!q.pending());
 // Duplicate retransmissions do not extend expiry or allocate a FIFO.
 assert(q.offer(77,2,1,0,0,2000));
 assert(q.offer(77,2,1,0,0,3000));
 assert(!q.due(4500,true,true,false)&&!q.pending());
 // Latest STOP supersedes older identity, without allowing stale ACK replay.
 assert(q.offer(100,2,1,0,0,5000));
 assert(q.offer(101,2,1,0,0,5010));
 assert(q.ticket().commandId==101&&q.due(5010,true,true,false));
 // Explicit session/transport fault purges outstanding evidence.
 assert(!q.due(5011,false,true,false)&&!q.pending());
 assert(q.offer(102,2,1,0,0,6000));
 assert(!q.due(6000,true,false,false)&&!q.pending());
 // Eight failed RF TXs are the maximum; heartbeat can resume afterwards.
 assert(q.offer(300,4,1,0,0,10000));
 for(unsigned i=0;i<StopAckOutbox::MAX_TX_ATTEMPTS;++i){
   const uint32_t now=10000U+i*StopAckOutbox::RETRY_GAP_MS;
   assert(q.due(now,true,true,false));q.admitted(now);q.completed(false);
 }
 assert(!q.pending());
 // A missing TX completion is kept busy, then its caller's 120ms PHY
 // watchdog must revoke readiness and clear the outbox.
 assert(q.offer(401,5,1,0,0,12000));
 assert(q.due(12000,true,true,false));q.admitted(12000);
 assert(!q.due(12200,true,true,false));
 assert(!q.due(12201,true,false,true)&&!q.pending());
 // 32-bit millis rollover and retry gap.
 assert(q.offer(500,6,1,0,0,0xfffffff0U));
 assert(q.due(0xfffffff0U,true,true,false));q.admitted(0xfffffff0U);
 q.completed(false);
 assert(!q.due(0x00000020U,true,true,false));
 assert(q.due(0x0000003BU,true,true,false));
 q.reset();assert(!q.pending());
 // Updating ACK result for the same STOP never restarts the TTL.
 assert(q.offer(600,7,0,0,0,0));
 assert(q.offer(600,7,1,1,2,100));
 assert(q.ticket().result==1&&q.ticket().stopState==1&&q.ticket().interlocks==2);
 assert(!q.due(2500,true,true,false));
 std::cout << "R30H-18 STOP ACK TX-busy outbox PASS (pure host, no radio)\n";
}
