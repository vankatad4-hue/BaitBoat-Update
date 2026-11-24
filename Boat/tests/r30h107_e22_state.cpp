#include "e22_tx_state.h"
#include <cassert>
using S=flytec_rtos::E22TxState;
int main(){
 S s;assert(s.outcome()==S::Outcome::Idle);
 s.accepted(100);s.poll(133,true);assert(s.pending());
 s.poll(134,false);assert(s.pending());
 s.poll(135,true);assert(s.outcome()==S::Outcome::LocalReady);
 s.accepted(200);s.poll(2199,false);assert(s.pending());
 s.poll(2200,true);assert(s.fault()&&!s.pending());
 s.poll(2300,true);assert(s.fault());s.reset();assert(!s.fault());
 s.uncertain();assert(s.fault());
 s.reset();s.accepted(UINT32_MAX-10);s.poll(23,true);
 assert(s.outcome()==S::Outcome::LocalReady);
}
