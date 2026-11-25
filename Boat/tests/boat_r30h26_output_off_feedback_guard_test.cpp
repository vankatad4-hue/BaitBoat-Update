// R30H-26: deterministic, non-actuating OFF-command versus LOCAL physical
// feedback guard. UINT32 rollover, settling windows, fresh-source contract.
#include "../source/boat_output_off_feedback_guard.h"
#include <cassert>
#include <cstdint>
#include <iostream>
using boat_r30h26::OutputOffFeedbackGuard;
static BoatHardwareSample evidence(uint32_t now){
 BoatHardwareSample s{};
 s.safetyAt=now;s.safetyValid=true;s.outputFeedbackValid=true;s.watchdogReady=true;
 s.stationary=true;s.releaseIdle=true;
 return s;
}
int main(){
 const uint32_t base=UINT32_MAX-150U;
 OutputOffFeedbackGuard g{};
 g.commandedDisable(base);
 auto s=evidence(base);s.leftMotorActive=true;
 assert(!g.motorStuck(s,base+249U));
 s.safetyAt=base+249U;
 assert(!g.mismatch(s,base+249U));
 // Repeated successful OFF writes MUST NOT move the original deadline.
 g.acceptedOutput({0,0,-1},base+248U);
 s.safetyAt=base+250U;
 assert(g.motorStuck(s,base+250U)&&g.mismatch(s,base+250U));
 s.leftMotorActive=false;s.rightMotorActive=true;
 assert(g.motorStuck(s,base+251U));
 s.rightMotorActive=false;
 assert(!g.mismatch(s,base+251U));
 s.releaseIdle=false;s.safetyAt=base+499U;
 assert(!g.coilStuck(s,base+499U));
 s.safetyAt=base+500U;
 assert(g.coilStuck(s,base+500U));
 s.outputFeedbackValid=false;
 assert(!g.mismatch(s,base+1000U)); // invalid sensor is separate STOP path
 s.outputFeedbackValid=true;s.safetyAt=base+900U;
 assert(!g.mismatch(s,base+1000U)); // stale feedback is not physical proof

 // Actual new actuator demand resets only the affected OFF timer.
 const uint32_t later=base+1200U;
 g.acceptedOutput({200,0,-1},later);
 s=evidence(later);s.leftMotorActive=true;
 assert(!g.motorStuck(s,later));
 g.acceptedOutput({0,0,-1},later+1U);
 s.safetyAt=later+250U;
 assert(!g.motorStuck(s,later+250U));
 s.safetyAt=later+251U;
 assert(g.motorStuck(s,later+251U));
 g.acceptedOutput({0,0,1},later+300U);
 s.releaseIdle=false;s.safetyAt=later+300U;
 assert(!g.coilStuck(s,later+300U));
 g.acceptedOutput({0,0,-1},later+301U);
 s.safetyAt=later+800U;
 assert(!g.coilStuck(s,later+800U));
 s.safetyAt=later+801U;
 assert(g.coilStuck(s,later+801U));
 std::cout<<"R30H-26 software OFF feedback monitor monotonic thresholds and rollover PASS (host only)\n";
}
