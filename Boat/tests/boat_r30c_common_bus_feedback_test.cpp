#include "../source/boat_common_bus_feedback_r30c.h"
#include "../source/boat_physical_driver_adapter.h"
#include <cassert>
#include <iostream>
using namespace boat_r30c;
static CommonBusFeedback hypothetical(uint32_t now) {
 CommonBusFeedback e{};
 e.stopCircuitHealthy=true;
 e.independentlySupervisedWatchdogHealthy=true;
 e.commonDisconnectClosedFeedback=true;
 e.commonDisconnectDcRatingSigned=true;
 e.commonDisconnectPowerOffTestSigned=true;
 e.coilOneHotPulseLimiterCommissioned=true;
 e.independentlyObservedCoilCurrentZero=true;
 e.independentlyObservedMotorCurrentZero=true;
 e.coilCurrentZero=true;
 e.motorCurrentZero=true;
 e.localBatteryHealthy=true;
 e.sampledAtMs=now;
 return e;
}
int main(){
 static_assert(boat_r29::SHARED_3S_PACK,"single common pack required");
 static_assert(!boat_r29::SEPARATE_MOTOR_SOLENOID_POWER,
               "motor and solenoid batteries are not separate");
 static_assert(!boat_r29::SHARED_BUS_PHYSICALLY_ACCEPTED,
               "high-current wiring remains uncommissioned");
 static_assert(!COMMON_BUS_HARDWARE_COMMISSIONED,
               "new feedback design cannot grant actuator power");
 const uint32_t t=10000U;
 CommonBusFeedback e=hypothetical(t);
 const BoatActuatorOutput off{0,0,-1}, drive{180,150,-1}, coil{0,0,1};
 assert(inspect(e,off,100,0,t)==Review::OFF_REQUEST);
 assert(inspect(e,drive,100,0,t)==Review::DESIGN_EVIDENCE_CONSISTENT);
 assert(inspect(e,coil,100,120,t)==Review::DESIGN_EVIDENCE_CONSISTENT);
 // This is hypothetical evidence in a host-only fixture, never permission.
 assert(!mayEnergizeActuators(e,drive,100,0,t));
 assert(!mayEnergizeActuators(e,coil,100,120,t));
 assert(inspect(e,{180,0,1},100,120,t)==Review::BAD_REQUEST);
 assert(inspect(e,{1001,0,-1},100,0,t)==Review::BAD_REQUEST);
 assert(inspect(e,drive,0,0,t)==Review::BAD_REQUEST);
 assert(inspect(e,drive,101,0,t)==Review::BAD_REQUEST);
 assert(inspect(e,coil,100,0,t)==Review::BAD_REQUEST);
 assert(inspect(e,coil,100,501,t)==Review::BAD_REQUEST);
 assert(inspect(e,drive,100,0,t+50)==Review::EXPIRED_FEEDBACK);
 auto bad=e;bad.stopCircuitHealthy=false;
 assert(inspect(bad,drive,100,0,t)==Review::STOP_OR_WATCHDOG_UNAVAILABLE);
 bad=e;bad.independentlySupervisedWatchdogHealthy=false;
 assert(inspect(bad,drive,100,0,t)==Review::STOP_OR_WATCHDOG_UNAVAILABLE);
 bad=e;bad.commonDisconnectClosedFeedback=false;
 assert(inspect(bad,drive,100,0,t)==Review::COMMON_DISCONNECT_UNQUALIFIED);
 bad=e;bad.commonDisconnectDcRatingSigned=false;
 assert(inspect(bad,drive,100,0,t)==Review::COMMON_DISCONNECT_UNQUALIFIED);
 bad=e;bad.commonDisconnectPowerOffTestSigned=false;
 assert(inspect(bad,drive,100,0,t)==Review::COMMON_DISCONNECT_UNQUALIFIED);
 bad=e;bad.localBatteryHealthy=false;
 assert(inspect(bad,drive,100,0,t)==Review::BATTERY_UNTRUSTED);
 bad=e;bad.coilOneHotPulseLimiterCommissioned=false;
 assert(inspect(bad,drive,100,0,t)==Review::RELEASE_LIMITER_UNQUALIFIED);
 bad=e;bad.independentlyObservedCoilCurrentZero=false;
 assert(inspect(bad,drive,100,0,t)==Review::MISSING_CURRENT_ZERO);
 bad=e;bad.independentlyObservedMotorCurrentZero=false;
 assert(inspect(bad,coil,100,120,t)==Review::MISSING_CURRENT_ZERO);
 bad=e;bad.coilCurrentZero=false;
 assert(inspect(bad,drive,100,0,t)==Review::MISSING_CURRENT_ZERO);
 bad=e;bad.motorCurrentZero=false;
 assert(inspect(bad,coil,100,120,t)==Review::MISSING_CURRENT_ZERO);
 // During active propulsion motor-current zero cannot be required; this is
 // why R30C checks motor-zero for coil REQUESTS, not drive REQUESTS.
 assert(inspect(bad,drive,100,0,t)==Review::DESIGN_EVIDENCE_CONSISTENT);
 // Signed in-memory booleans cannot trick the physical driver into working.
 BoatPhysicalDriverAdapter actual;
 BoatPhysicalHal missing{};
 const uint16_t pulseWidths[4]={100,100,100,100};
 assert(!actual.begin(missing,pulseWidths,false));
 assert(!mayEnergizeActuators(e,drive,100,0,t));
 // millis wrap: recent evidence is fresh; old evidence must be rejected.
 bad=hypothetical(0xfffffffeU);
 assert(inspect(bad,drive,100,0,1U)==Review::DESIGN_EVIDENCE_CONSISTENT);
 assert(inspect(bad,drive,100,0,100U)==Review::EXPIRED_FEEDBACK);
 std::cout<<"R30C common-bus STOP/watchdog/contact/current-zero model PASS; physical output permanently locked\n";
}
