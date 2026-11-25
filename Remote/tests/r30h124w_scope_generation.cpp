#include "remote_mission_tx_scope.h"
#include "tx_generation_gate.h"
#include <cassert>
using namespace flytec_rtos;
static bool admit(void* p){++*static_cast<unsigned*>(p);return true;}
int main(){
 MissionTxScope scope;TxGenerationGate gate;unsigned starts=0;
 using R=TxGenerationGate::Result;
 assert(gate.latchStop(1));assert(gate.stopped());
 scope.preparationQueued(9,false,false);scope.preparationQueued(9,true,false);
 assert(scope.handoff(9,true,true));scope.clearQueued(21);
 assert(!scope.permitsMission(9,true));assert(scope.confirmClear(9,21,true,true));
 assert(gate.stopped()&&scope.permitsMission(9,true));
 assert(gate.tryAdmit(1,admit,&starts,false)==R::Inhibited&&starts==0);
 assert(gate.tryAdmit(1,admit,&starts,scope.permitsMission(9,true))==R::Accepted&&starts==1);
 assert(gate.latchStop(2));scope.resetAfterRevocation();
 assert(gate.tryAdmit(1,admit,&starts,true)==R::Stale&&starts==1);
 assert(!scope.permitsMission(9,true)&&gate.stopped());
 assert(gate.tryAdmit(2,admit,&starts,false)==R::Inhibited&&starts==1);
}
