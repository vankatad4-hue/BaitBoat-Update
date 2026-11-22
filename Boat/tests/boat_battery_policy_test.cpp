#include "../source/boat_battery_policy.h"
#include <cassert>
#include <iostream>
int main(){
 BoatBatteryMonitor m;
 auto s=m.update(true,12000,0);assert(s.valid&&s.band==BOAT_BATTERY_GREEN&&s.percent==80&&!s.rthRequested);
 m.reset();s=m.update(true,11300,0);assert(s.band==BOAT_BATTERY_YELLOW&&s.percent<50);
 m.reset();s=m.update(true,10700,100);assert(s.band==BOAT_BATTERY_RED&&s.lowWarning&&!s.rthRequested&&!s.critical);
 s=m.update(true,10700,3099);assert(!s.rthRequested);
 s=m.update(true,10700,3100);assert(s.rthRequested&&s.band==BOAT_BATTERY_RED&&!s.critical);
 s=m.update(true,12000,3200);assert(s.rthRequested&&s.band==BOAT_BATTERY_RED);
 m.reset();s=m.update(true,10000,0);assert(s.band==BOAT_BATTERY_RED&&!s.critical);
 s=m.update(true,9000,20);assert(s.critical&&s.band==BOAT_BATTERY_CRITICAL&&!s.rthRequested);
 m.reset();s=m.update(true,12000,0);assert(s.valid);
 s=m.snapshot(1001);assert(!s.valid&&s.percent==255&&s.band==BOAT_BATTERY_UNKNOWN);
 s=m.update(false,0,1002);assert(!s.valid);
 std::cout<<"3S battery estimate, warning, RTH latch and critical STOP passed\n";
}
