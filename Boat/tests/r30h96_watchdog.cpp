#include "radio_watchdog.h"
#include <cassert>
using W=flytec_rtos::RadioWatchdog;
int main(){
 W w(500);assert(!w.permit());
 w.reset(100,0);assert(w.update(101,0,true,false)==W::State::Starting);
 assert(w.update(110,1,true,false)==W::State::Healthy && w.permit());
 assert(w.update(609,1,true,false)==W::State::Healthy);
 assert(w.update(610,2,true,false)==W::State::Fault && !w.permit());
 assert(w.update(611,3,true,false)==W::State::Fault);
 w.reset(700,3);assert(w.update(701,4,true,false)==W::State::Healthy);
 assert(w.update(702,5,false,false)==W::State::Fault);
 w.reset(800,5);assert(w.update(801,6,true,true)==W::State::Fault);
 w.reset(0xffffff00U,0xffffffffU);
 assert(w.update(0xffffff01U,0,true,false)==W::State::Healthy);
 assert(w.update(0x000000f4U,0,true,false)==W::State::Healthy);
 assert(w.update(0x000000f5U,0,true,false)==W::State::Fault);
}
