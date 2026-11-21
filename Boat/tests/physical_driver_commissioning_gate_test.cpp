#include <cassert>
#include "../source/physical_driver_commissioning_gate.h"
int main(){
 BoatSafetyContacts c{};
 assert(!boatPhysicalOutputsPermitted(c,100));
 c={true,true,true,true,true,true,true,true,100};
 assert(boatPhysicalOutputsPermitted(c,100));
 assert(boatPhysicalCoilPulsePermitted(c,100,0,300,true));
 assert(!boatPhysicalCoilPulsePermitted(c,100,1,501,true));
 assert(!boatPhysicalCoilPulsePermitted(c,100,1,200,false));
 assert(!boatPhysicalOutputsPermitted(c,201));
 c.sampledAt=200;c.watchdogContactClosed=false;
 assert(!boatPhysicalOutputsPermitted(c,200));
 c.watchdogContactClosed=true;c.coilCurrentZero=false;
 assert(!boatPhysicalCoilPulsePermitted(c,200,2,100,true));
 BoatHardwareSample sample{};
 boatPhysicalSafetyToSample(c,200,sample);
 assert(!sample.releaseIdle&&sample.watchdogReady);
 boatPhysicalSafetyToSample(c,400,sample);
 assert(!sample.safetyValid&&!sample.watchdogReady);
}
