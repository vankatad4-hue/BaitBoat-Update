#include "../source/boat_common_bus_readonly_port_r30d.h"
#include "../source/boat_physical_driver_adapter.h"
#include <cassert>
#include <iostream>

using namespace boat_r30d;
struct FakeInputs final : ReadOnlySource {
 RawFrame next{};
 bool success=true;
 unsigned readCount=0;
 bool read(RawFrame &out) override {
  ++readCount;
  if(!success)return false;
  out=next;
  return true;
 }
};
static RawFrame nominal(uint32_t at){
 RawFrame f{};
 f.stop={true,at,1};
 f.watchdog={true,at,2};
 f.commonDisconnectAux={true,at,3};
 f.coilPulseLimiter={true,at,4};
 f.coilCurrentZero={true,at,5};
 f.motorCurrentZero={true,at,6};
 f.battery={true,12200U,at,7};
 return f;
}
static bool denied(const Capture &c){
 return !c.canAuthorizeActuators &&
    !boat_r30c::mayEnergizeActuators(c.feedback,{250,250,-1},100,0,c.capturedAtMs);
}
int main(){
 static_assert(!ACTUAL_READER_COMMISSIONED, "No installed R30D sampler");
 static_assert(!boat_r30c::COMMON_BUS_HARDWARE_COMMISSIONED, "No cutoff hardware");
 static_assert(!boat_r29::SHARED_BUS_PHYSICALLY_ACCEPTED, "No high-current load wiring");
 static_assert(!boat_r27::INA231_PHYSICAL_COMMISSIONED, "Sensor not physically signed");
 static_assert(!boat_r29::SEPARATE_MOTOR_SOLENOID_POWER, "One common 3S bus");

 ReadOnlyCommonBusPort probe;
 FakeInputs reader{};
 const uint32_t at=1000;
 auto before=probe.snapshot(at);
 assert(before.result==Result::NEVER_READ&&!before.observed&&denied(before));
 reader.next=nominal(at);
 const Capture healthy=probe.sample(reader,at);
 assert(reader.readCount==1);
 assert(healthy.result==Result::OK_DIAGNOSTIC_ONLY&&healthy.observed);
 assert(healthy.raw.stop.asserted && healthy.raw.coilPulseLimiter.asserted);
 assert(healthy.raw.battery.millivolts==12200U);
 assert(healthy.feedback.stopCircuitHealthy);
 assert(healthy.feedback.independentlySupervisedWatchdogHealthy);
 assert(healthy.feedback.commonDisconnectClosedFeedback);
 assert(healthy.feedback.coilCurrentZero && healthy.feedback.motorCurrentZero);
 assert(healthy.feedback.sampledAtMs==at);
 assert(!healthy.feedback.coilOneHotPulseLimiterCommissioned);
 assert(!healthy.feedback.commonDisconnectDcRatingSigned);
 assert(!healthy.feedback.commonDisconnectPowerOffTestSigned);
 assert(!healthy.feedback.independentlyObservedCoilCurrentZero);
 assert(!healthy.feedback.independentlyObservedMotorCurrentZero);
 assert(!healthy.feedback.localBatteryHealthy);
 assert(denied(healthy));
 assert(boat_r30c::inspect(healthy.feedback,{200,200,-1},100,0,at) ==
        boat_r30c::Review::COMMON_DISCONNECT_UNQUALIFIED);

 // A new failure invalidates last successful diagnostic snapshot.
 reader.success=false;
 auto bad=probe.sample(reader,1001);
 assert(bad.result==Result::IO_FAILURE&&!bad.observed&&denied(bad));
 assert(!probe.snapshot(1002).observed);
 reader.success=true;reader.next=nominal(1010);
 // Every one of seven feedback channels must have its own nonzero origin ID.
 reader.next.motorCurrentZero.sourceId=0;
 assert(probe.sample(reader,1010).result==Result::SOURCE_UNASSIGNED);
 reader.next=nominal(1010);
 reader.next.motorCurrentZero.sourceId=reader.next.coilCurrentZero.sourceId;
 assert(probe.sample(reader,1010).result==Result::DUPLICATE_SOURCE);
 reader.next=nominal(1010);
 reader.next.battery.sourceId=reader.next.watchdog.sourceId;
 assert(probe.sample(reader,1010).result==Result::DUPLICATE_SOURCE);

 // A reader must provide original acquisition timestamps, not poll() time.
 reader.next=nominal(1000);
 assert(probe.sample(reader,1049).result==Result::OK_DIAGNOSTIC_ONLY);
 assert(probe.sample(reader,1050).result==Result::STALE_SAFETY);
 assert(!probe.snapshot(1050).observed);
 reader.next=nominal(1000);
 reader.next.watchdog.acquiredAtMs=1051; // invented future time rejected
 assert(probe.sample(reader,1050).result==Result::STALE_SAFETY);
 reader.next=nominal(1050);
 reader.next.battery.acquiredAtMs=49;
 assert(probe.sample(reader,1050).result==Result::STALE_BATTERY);
 reader.next=nominal(1050);
 reader.next.battery.millivolts=0;
 assert(probe.sample(reader,1050).result==Result::INVALID_BATTERY);
 reader.next.battery.millivolts=13100;
 assert(probe.sample(reader,1050).result==Result::INVALID_BATTERY);
 reader.next.battery.millivolts=9900; reader.next.battery.valid=false;
 assert(probe.sample(reader,1050).result==Result::INVALID_BATTERY);

 // Valid acquisition of an UNHEALTHY contact is still observable, never
 // treated as a healthy or commissioned contact.
 reader.next=nominal(1080);
 reader.next.stop.asserted=false;
 auto fault=probe.sample(reader,1080);
 assert(fault.result==Result::OK_DIAGNOSTIC_ONLY&&fault.observed);
 assert(!fault.feedback.stopCircuitHealthy&&denied(fault));
 assert(boat_r30c::inspect(fault.feedback,{200,200,-1},100,0,1080) ==
        boat_r30c::Review::STOP_OR_WATCHDOG_UNAVAILABLE);
 reader.next=nominal(1090);
 reader.next.watchdog.asserted=false;
 fault=probe.sample(reader,1090);
 assert(!fault.feedback.independentlySupervisedWatchdogHealthy&&denied(fault));
 reader.next=nominal(1100);
 reader.next.commonDisconnectAux.asserted=false;
 fault=probe.sample(reader,1100);
 assert(!fault.feedback.commonDisconnectClosedFeedback&&denied(fault));
 reader.next=nominal(1110);
 reader.next.coilCurrentZero.asserted=false;
 fault=probe.sample(reader,1110);
 assert(!fault.feedback.coilCurrentZero&&denied(fault));
 reader.next=nominal(1120);
 reader.next.motorCurrentZero.asserted=false;
 fault=probe.sample(reader,1120);
 assert(!fault.feedback.motorCurrentZero&&denied(fault));
 reader.next=nominal(1130);
 reader.next.coilPulseLimiter.asserted=false;
 fault=probe.sample(reader,1130);
 assert(!fault.raw.coilPulseLimiter.asserted &&
        !fault.feedback.coilOneHotPulseLimiterCommissioned&&denied(fault));

 // A cached response with 30 ms-old contact must expire after the actual
 // acquisition age reaches 50 ms, despite a more recent reader invocation.
 reader.next=nominal(1200);
 reader.next.coilCurrentZero.acquiredAtMs=1170;
 auto held=probe.sample(reader,1200);
 assert(held.observed && held.feedback.sampledAtMs==1170);
 assert(probe.snapshot(1219).observed);
 assert(probe.snapshot(1220).result==Result::STALE_SAFETY);
 // Millis wrap with 3ms elapsed is fresh, 50ms expired.
 reader.next=nominal(0xfffffffeU);
 auto rollover=probe.sample(reader,1U);
 assert(rollover.observed&&rollover.feedback.sampledAtMs==0xfffffffeU);
 assert(probe.snapshot(47U).observed);
 assert(probe.snapshot(48U).result==Result::STALE_SAFETY);
 // An unsupported physical driver is not attached or made active.
 BoatPhysicalDriverAdapter physical;
 BoatPhysicalHal nonexistent{};
 const uint16_t pulses[4]={100,100,100,100};
 assert(!physical.begin(nonexistent,pulses,false));
 probe.clear();
 assert(probe.snapshot(2000).result==Result::NEVER_READ);
 std::cout << "R30D independent read-only common-bus input faults PASS; no GPIO/output authority\n";
 return 0;
}
