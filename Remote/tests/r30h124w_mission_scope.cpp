#include "remote_mission_tx_scope.h"
#include <cassert>
using namespace flytec_rtos;
int main(){
 MissionTxScope s;using S=MissionTxScope::State;
 assert(!s.locked()&&!s.permitsMission(9,true)&&!s.permitsPrepare(9,false,false));
 assert(s.permitsPrepare(9,true,false));s.preparationQueued(9,false,false);
 assert(s.locked()&&!s.permitsPrepare(10,true,false)&&!s.handoff(9,true,true));
 assert(s.permitsPrepare(9,false,false));s.preparationQueued(9,true,false);
 assert(!s.handoff(10,true,true)&&!s.handoff(9,false,true)&&!s.handoff(9,true,false));
 assert(s.handoff(9,true,true)&&s.state()==S::HandedOff);
 assert(!s.permitsMission(9,true)&&!s.permitsClear(10,true)&&!s.permitsClear(9,false));
 assert(s.permitsClear(9,true));s.clearQueued(21);
 assert(!s.permitsClear(9,true)&&!s.permitsMission(9,true));
 assert(!s.confirmClear(9,21,false,true)&&!s.confirmClear(9,21,true,false));
 assert(!s.confirmClear(10,21,true,true)&&!s.confirmClear(9,22,true,true));
 assert(s.confirmClear(9,21,true,true)&&s.permitsMission(9,true));
 assert(!s.permitsMission(10,true)&&!s.permitsMission(9,false)&&!s.permitsPrepare(9,true,false));
 s.resetAfterRevocation();assert(!s.locked()&&!s.permitsMission(9,true)&&!s.confirmClear(9,21,true,true));
 assert(s.permitsPrepare(10,true,false));s.preparationQueued(10,false,false);
 assert(s.permitsPrepare(10,false,true));s.preparationQueued(10,false,true);
 assert(s.state()==S::Canceling&&s.permitsPrepare(10,false,true)&&!s.permitsPrepare(10,true,false)&&!s.handoff(10,true,true));
}
