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
 std::cout<<"Mission remote control ACK handling passed\n";
}
