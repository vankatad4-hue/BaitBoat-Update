#pragma once
#include <stdint.h>
#include "mdd20a_direct_profile.h"
#include "carrier_output_plan.h"
// These are physically measured local feedback values, not remote commands.
// The separate electrical STOP/watchdog cutoff must work without the ESP32.
struct Mdd20aDirectFeedback {
 uint32_t sampleAtMs;
 bool trusted,physicalStopReleased,watchdogHealthy,gateFeedbackOn;
 bool batteryHealthy,motorsIdle,solenoidsIdle,interlocksHealthy;
};
inline bool mdd20aDirectFeedbackReady(const Mdd20aDirectFeedback &s,uint32_t now) {
 return s.trusted&&now-s.sampleAtMs<50U&&s.physicalStopReleased&&
  s.watchdogHealthy&&s.gateFeedbackOn&&s.batteryHealthy&&
  s.solenoidsIdle&&s.interlocksHealthy;
}
class Mdd20aDirectSafety {
public:
 bool configure(const Mdd20aDirectProfile &p,uint64_t allowed,bool reverseL,bool reverseR,
                uint16_t pwmMax,uint16_t reverseDeadtimeMs) {
  fault_=true;
  if(!mdd20aDirectProfileValid(p,allowed))return false;
  if(!planner_.configure(true,reverseL,reverseR,pwmMax,reverseDeadtimeMs))return false;
  fault_=false;return true;
 }
 void fault(){fault_=true;planner_.reset();}
 // This policy NEVER switches/arms the independent hardware gate. The gate
 // must have already been explicitly commissioned and physically confirmed.
 bool prepare(const BoatActuatorOutput &request,const Mdd20aDirectFeedback &fb,
              uint32_t now,CarrierOutputWords &out) {
  out={};
  if(fault_||!mdd20aDirectFeedbackReady(fb,now)) {fault();return false;}
  // Four solenoids require their own hardware one-hot/time-limited driver.
  if(request.releaseChannel!=-1&&!fb.motorsIdle){fault();return false;}
  // The shared output planner enforces no switching from a moving motor to
  // a coil, and no direct restart of propulsion after a coil pulse.
  if(!planner_.convert(request,now,fb.interlocksHealthy,out)) {fault();out={};return false;}
  return true;
 }
 bool failed() const{return fault_;}
private:
 bool fault_=true;
 CarrierOutputPlan planner_;
};
