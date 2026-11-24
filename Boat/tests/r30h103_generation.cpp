#include "tx_generation_gate.h"
#include <cassert>
using G=flytec_rtos::TxGenerationGate;
static bool accept(void* p){++*static_cast<unsigned*>(p);return true;}
int main(){
 G g;unsigned n=0;
 assert(g.tryAdmit(0,accept,&n)==G::Result::Stale);
 assert(g.transition(1));assert(g.tryAdmit(1,accept,&n)==G::Result::Accepted);
 assert(g.transition(2));assert(!g.transition(1));
 assert(g.tryAdmit(1,accept,&n)==G::Result::Stale&&n==1);
 assert(g.tryAdmit(2,accept,&n)==G::Result::Accepted&&n==2);
 assert(g.transition(UINT32_MAX));assert(!g.transition(0));
}
