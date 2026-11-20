#include "../source/carrier_master_port.h"
#include <cassert>
#include <iostream>
uint32_t fakeNow=0;
int main(){
 TwoWire wire;const uint16_t cap[4]={100,100,100,100};wire.core.configure(true,cap);
 wire.inputs={uint8_t(carrier::STOP_OK|carrier::WATCHDOG_OK|carrier::INTERLOCK_OK|carrier::LEFT_IDLE|carrier::RIGHT_IDLE|carrier::COILS_IDLE|carrier::FEEDBACK_OK),15};
 CarrierMasterPort master(wire);carrier::Status status{};
 assert(!master.command(100,100,-1));assert(master.syncOff()&&master.readStatus(status)&&status.state==carrier::READY);
 assert(!master.command(100,100,-1));
 wire.inputs.flags|=carrier::GATE_ON;fakeNow=10;assert(master.command(0,0,-1)&&master.readStatus(status)&&status.state==carrier::ARMED);
 assert(master.command(100,100,-1)&&master.readStatus(status));
 wire.corrupt=true;assert(!master.readStatus(status)&&!master.command(100,100,-1));wire.corrupt=false;
 fakeNow=60;assert(!wire.core.tick(fakeNow,wire.inputs).permit);
 wire.inputs.flags&=~carrier::GATE_ON;assert(master.syncOff()&&master.readStatus(status));
 wire.freeze=true;assert(!master.readStatus(status));wire.freeze=false;
 wire.failWrite=true;assert(!master.command(0,0,-1));wire.failWrite=false;
 assert(master.disarm());assert(!wire.core.tick(fakeNow,wire.inputs).permit);
 std::cout<<"Actual Tracker master transport against simulated Pico receiver passed\n";
}
