#include "../source/mission_control_client.h"
#include <cassert>
#include <iostream>
int main(){
 MissionControlClient c;MissionControlRequest r{};
 assert(!c.request(MCTRL_START,0));c.bind(5);assert(c.locked());
 assert(c.observe({5,MC_PREPARED,0,0,MC_STATUS},1,true));
 assert(c.request(MCTRL_START,2)&&c.next(2,r));
 assert(!c.observe({5,MC_OUTBOUND,MCTRL_START,(uint16_t)(r.request+1),MC_ACCEPTED},3,true));
 assert(!c.observe({5,MC_OUTBOUND,MCTRL_START,r.request,MC_ACCEPTED},3,false));
 assert(c.next(402,r)&&r.request==1);
 assert(c.observe({5,MC_OUTBOUND,MCTRL_START,r.request,MC_ACCEPTED},403,true)&&!c.pending());
 assert(c.request(MCTRL_PAUSE,500)&&c.next(500,r)&&c.next(900,r)&&c.next(1300,r));
 assert(!c.next(1700,r)&&c.unknown());assert(!c.request(MCTRL_RESUME,1701));
 assert(c.request(MCTRL_CANCEL,1701)&&c.next(1701,r));
 assert(c.observe({5,MC_ABORTED,MCTRL_CANCEL,r.request,MC_ACCEPTED},1702,true)&&!c.locked());
 assert(!c.fresh(2702));c.reset();assert(!c.locked());
 // A blocked E22 slot must not consume a mission attempt.
 MissionControlClient busy;MissionControlRequest due{};
 busy.bind(11);assert(busy.request(MCTRL_PAUSE,100));
 assert(busy.due(100,due,true)&&busy.due(200,due,true));
 busy.admitted(200);
 assert(!busy.due(3199,due,true));
 assert(busy.due(3200,due,true)); // no second admission yet
 assert(busy.due(3500,due,true));
 busy.admitted(3500);
 assert(!busy.due(6499,due,true));
 assert(busy.due(6500,due,true));
 // Recovering nRF after a LoRa attempt must not instantly expire the
 // in-flight correlated request under the primary-only 1.8-second limit.
 MissionControlClient failover;MissionControlRequest pending{};
 failover.bind(21);assert(failover.request(MCTRL_PAUSE,100));
 assert(failover.due(100,pending,true));failover.admitted(100,true);
 assert(failover.due(2200,pending,false)&&failover.pending()&&!failover.unknown());
 assert(failover.due(3100,pending,false));failover.admitted(3100,false);
 assert(failover.observe({21,MC_PAUSED,MCTRL_PAUSE,pending.request,MC_ACCEPTED},3101,true));
 assert(!failover.pending()&&!failover.unknown());
 std::cout<<"Mission remote control ACK handling passed\n";
}
