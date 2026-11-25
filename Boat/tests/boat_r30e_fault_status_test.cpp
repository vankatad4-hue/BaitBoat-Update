#include "../source/boat_fault_status.h"
#include "../source/boat_physical_driver_adapter.h"
#include <cassert>
#include <cstring>
#include <iostream>
using namespace boat_r30d;
using namespace boat_r30e;
struct Fake : ReadOnlySource {
 RawFrame v{};bool ok=true;
 bool read(RawFrame &r)override{if(!ok)return false;r=v;return true;}
};
static RawFrame allHealthy(uint32_t t) {
 RawFrame f{};
 f.stop={true,t,1};f.watchdog={true,t,2};
 f.commonDisconnectAux={true,t,3};f.coilPulseLimiter={true,t,4};
 f.coilCurrentZero={true,t,5};f.motorCurrentZero={true,t,6};
 f.battery={true,12000U,t,7};
 return f;
}
static void locked(const Status &s){
 assert(!s.safeToActuate);
 assert((s.flags&UNVERIFIED)!=0 && (s.flags&COMMISSIONING_LOCK)!=0);
}
int main(){
 static_assert(!ACTUAL_READER_COMMISSIONED,"read-only fake is not physical");
 static_assert(!boat_r30c::COMMON_BUS_HARDWARE_COMMISSIONED,"no shared-bus cutoff");
 static_assert(!boat_r29::SHARED_BUS_PHYSICALLY_ACCEPTED,"VM/VH not commissioned");
 const uint32_t t=1000;
 ReadOnlyCommonBusPort port;
 const Status absent=makeStatus(port.snapshot(t),t,1);
 assert(absent.reason==Fault::NO_READER);
 assert((absent.flags&SOURCE_FAULT)!=0&&absent.millivolts==0xffffU);
 locked(absent);
 uint8_t bytes[8]{};
 encodeLocal(absent,bytes);
 assert(bytes[0]==0xD5 && bytes[1]==1);
 assert(bytes[2]==static_cast<uint8_t>(Fault::NO_READER));
 assert(bytes[3]==(uint8_t)absent.flags && bytes[4]==(uint8_t)(absent.flags>>8));
 assert(bytes[5]==0xff&&bytes[6]==0xff&&bytes[7]==1);
 assert(std::strcmp(label(absent.reason),"NO_READER")==0);
 Fake input;input.v=allHealthy(t);
 auto capture=port.sample(input,t);
 assert(capture.observed);
 auto status=makeStatus(capture,t,2);
 assert(status.reason==Fault::UNCOMMISSIONED);
 assert(status.millivolts==12000U && (status.flags&OBSERVED)!=0);
 locked(status);
 encodeLocal(status,bytes);
 assert(bytes[5]==0xe0 && bytes[6]==0x2e && bytes[7]==2);
 assert(!boat_r30c::mayEnergizeActuators(capture.feedback,{300,300,-1},100,0,t));
 // Prioritize STOP and keep every simultaneous fault in flags.
 input.v=allHealthy(t+10);
 input.v.stop.asserted=false;
 input.v.watchdog.asserted=false;
 input.v.commonDisconnectAux.asserted=false;
 capture=port.sample(input,t+10);
 status=makeStatus(capture,t+10);
 assert(status.reason==Fault::STOP_OPEN);
 assert((status.flags&(STOP_FAULT|WATCHDOG_FAULT|DISCONNECT_FAULT))==
        (STOP_FAULT|WATCHDOG_FAULT|DISCONNECT_FAULT));
 locked(status);
 input.v=allHealthy(t+20);input.v.watchdog.asserted=false;
 status=makeStatus(port.sample(input,t+20),t+20);
 assert(status.reason==Fault::WATCHDOG_LOST);locked(status);
 input.v=allHealthy(t+30);input.v.commonDisconnectAux.asserted=false;
 status=makeStatus(port.sample(input,t+30),t+30);
 assert(status.reason==Fault::DISCONNECT_OPEN);locked(status);
 input.v=allHealthy(t+40);input.v.coilPulseLimiter.asserted=false;
 status=makeStatus(port.sample(input,t+40),t+40);
 assert(status.reason==Fault::LIMITER_NOT_READY);locked(status);
 input.v=allHealthy(t+50);input.v.coilCurrentZero.asserted=false;
 status=makeStatus(port.sample(input,t+50),t+50);
 assert(status.reason==Fault::COIL_CURRENT_NONZERO);locked(status);
 input.v=allHealthy(t+60);input.v.motorCurrentZero.asserted=false;
 status=makeStatus(port.sample(input,t+60),t+60);
 assert(status.reason==Fault::MOTOR_CURRENT_NONZERO);locked(status);
 input.v=allHealthy(t+70);input.ok=false;
 status=makeStatus(port.sample(input,t+70),t+70);
 assert(status.reason==Fault::READ_FAILURE);
 assert((status.flags&SOURCE_FAULT)!=0&&!(status.flags&OBSERVED));
 assert(status.millivolts==0xffffU);locked(status);
 input.ok=true;input.v=allHealthy(t+80);input.v.stop.sourceId=0;
 status=makeStatus(port.sample(input,t+80),t+80);
 assert(status.reason==Fault::SOURCE_UNASSIGNED);locked(status);
 input.v=allHealthy(t+90);input.v.stop.sourceId=2;
 status=makeStatus(port.sample(input,t+90),t+90);
 assert(status.reason==Fault::SOURCE_DUPLICATED);locked(status);
 input.v=allHealthy(t+100);input.v.stop.acquiredAtMs=t+50;
 status=makeStatus(port.sample(input,t+100),t+100);
 assert(status.reason==Fault::CONTACT_EXPIRED);
 assert((status.flags&STALE_FAULT)!=0);locked(status);
 input.v=allHealthy(t+110);input.v.battery.acquiredAtMs=100;
 status=makeStatus(port.sample(input,t+110),t+110);
 assert(status.reason==Fault::BATTERY_EXPIRED);
 assert((status.flags&BATTERY_FAULT)!=0);locked(status);
 input.v=allHealthy(t+120);input.v.battery.millivolts=14000;
 status=makeStatus(port.sample(input,t+120),t+120);
 assert(status.reason==Fault::BATTERY_INVALID);locked(status);
 input.v=allHealthy(t+130);
 capture=port.sample(input,t+130);
 status=makeStatus(capture,t+180); // caller kept cached copy
 assert(status.reason==Fault::CONTACT_EXPIRED&&status.millivolts==0xffffU);
 locked(status);
 ChangeTracker tracker;
 assert(tracker.observe(absent));
 assert(!tracker.observe(absent));
 assert(tracker.observe(status));
 assert(!tracker.observe(status));
 assert(tracker.events()==2U);
 tracker.reset();
 assert(tracker.events()==0U && tracker.observe(absent));
 // A forged diagnostic status may not be used as physical output feedback.
 BoatPhysicalDriverAdapter physical;
 BoatPhysicalHal unavailable{};
 const uint16_t pulses[4]={100,100,100,100};
 assert(!physical.begin(unavailable,pulses,false));
 std::cout<<"R30E diagnostic reason flags local envelope cache-fault telemetry PASS; physical driver disabled\n";
}
