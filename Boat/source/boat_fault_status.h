#pragma once
#include <stdint.h>
#include "boat_common_bus_readonly_port.h"

// R30E diagnostic-only status and local telemetry envelope.
// There is deliberately NO radio payload kind, actuator command or GPIO API.
// The current Boat firmware has no commissioned R30D reader and therefore
// produces explicit NO_READER reports rather than simulated healthy inputs.
namespace boat_r30e {
enum class Fault : uint8_t {
 NO_READER=1, READ_FAILURE, SOURCE_UNASSIGNED, SOURCE_DUPLICATED,
 CONTACT_EXPIRED, BATTERY_EXPIRED, BATTERY_INVALID,
 STOP_OPEN, WATCHDOG_LOST, DISCONNECT_OPEN, LIMITER_NOT_READY,
 COIL_CURRENT_NONZERO, MOTOR_CURRENT_NONZERO,
 UNCOMMISSIONED
};
enum Flag : uint16_t {
 OBSERVED=0x0001,
 UNVERIFIED=0x0002,
 STOP_FAULT=0x0004,
 WATCHDOG_FAULT=0x0008,
 DISCONNECT_FAULT=0x0010,
 LIMITER_FAULT=0x0020,
 COIL_ZERO_FAULT=0x0040,
 MOTOR_ZERO_FAULT=0x0080,
 BATTERY_FAULT=0x0100,
 SOURCE_FAULT=0x0200,
 STALE_FAULT=0x0400,
 COMMISSIONING_LOCK=0x0800
};
struct Status {
 Fault reason=Fault::NO_READER;
 uint16_t flags=UNVERIFIED|COMMISSIONING_LOCK;
 uint16_t millivolts=0xffffU; // sentinel for no validated fresh sample
 uint32_t atMs=0;
 uint8_t sequence=0;
 bool safeToActuate=false; // no diagnostic report can authorize outputs
};
inline const char *label(Fault reason) {
 switch(reason) {
  case Fault::NO_READER:return "NO_READER";
  case Fault::READ_FAILURE:return "READ_FAILURE";
  case Fault::SOURCE_UNASSIGNED:return "SOURCE_UNASSIGNED";
  case Fault::SOURCE_DUPLICATED:return "SOURCE_DUPLICATED";
  case Fault::CONTACT_EXPIRED:return "CONTACT_EXPIRED";
  case Fault::BATTERY_EXPIRED:return "BATTERY_EXPIRED";
  case Fault::BATTERY_INVALID:return "BATTERY_INVALID";
  case Fault::STOP_OPEN:return "STOP_OPEN";
  case Fault::WATCHDOG_LOST:return "WATCHDOG_LOST";
  case Fault::DISCONNECT_OPEN:return "DISCONNECT_OPEN";
  case Fault::LIMITER_NOT_READY:return "LIMITER_NOT_READY";
  case Fault::COIL_CURRENT_NONZERO:return "COIL_CURRENT_NONZERO";
  case Fault::MOTOR_CURRENT_NONZERO:return "MOTOR_CURRENT_NONZERO";
  case Fault::UNCOMMISSIONED:return "UNCOMMISSIONED";
 }
 return "UNKNOWN_FAULT";
}
inline Status makeStatus(const boat_r30d::Capture &capture,
                         uint32_t nowMs,uint8_t sequence=0) {
 Status r{};
 r.atMs=nowMs;r.sequence=sequence;
 // Always preserve the immutable physical commissioning lock.
 r.flags=UNVERIFIED|COMMISSIONING_LOCK;
 if (!capture.observed) {
  switch(capture.result) {
   case boat_r30d::Result::NEVER_READ:
    r.reason=Fault::NO_READER;r.flags|=SOURCE_FAULT;break;
   case boat_r30d::Result::IO_FAILURE:
    r.reason=Fault::READ_FAILURE;r.flags|=SOURCE_FAULT;break;
   case boat_r30d::Result::SOURCE_UNASSIGNED:
    r.reason=Fault::SOURCE_UNASSIGNED;r.flags|=SOURCE_FAULT;break;
   case boat_r30d::Result::DUPLICATE_SOURCE:
    r.reason=Fault::SOURCE_DUPLICATED;r.flags|=SOURCE_FAULT;break;
   case boat_r30d::Result::STALE_SAFETY:
    r.reason=Fault::CONTACT_EXPIRED;r.flags|=STALE_FAULT;break;
   case boat_r30d::Result::STALE_BATTERY:
    r.reason=Fault::BATTERY_EXPIRED;r.flags|=BATTERY_FAULT|STALE_FAULT;break;
   case boat_r30d::Result::INVALID_BATTERY:
    r.reason=Fault::BATTERY_INVALID;r.flags|=BATTERY_FAULT;break;
   default:r.reason=Fault::READ_FAILURE;r.flags|=SOURCE_FAULT;break;
  }
  return r;
 }
 if (capture.result!=boat_r30d::Result::OK_DIAGNOSTIC_ONLY ||
     (uint32_t)(nowMs-capture.feedback.sampledAtMs) >=
      boat_r30c::MAX_FEEDBACK_AGE_MS) {
  r.reason=Fault::CONTACT_EXPIRED;r.flags|=STALE_FAULT;return r;
 }
 if ((uint32_t)(nowMs-capture.raw.battery.acquiredAtMs) >
      boat_r30d::MAX_BATTERY_AGE_MS) {
  r.reason=Fault::BATTERY_EXPIRED;r.flags|=STALE_FAULT|BATTERY_FAULT;return r;
 }
 if (!capture.raw.battery.valid ||
     capture.raw.battery.millivolts<9000U ||
     capture.raw.battery.millivolts>13000U) {
  r.reason=Fault::BATTERY_INVALID;r.flags|=BATTERY_FAULT;return r;
 }
 r.flags|=OBSERVED;
 r.millivolts=capture.raw.battery.millivolts;
 const auto &f=capture.raw;
 if (!f.stop.asserted)r.flags|=STOP_FAULT;
 if (!f.watchdog.asserted)r.flags|=WATCHDOG_FAULT;
 if (!f.commonDisconnectAux.asserted)r.flags|=DISCONNECT_FAULT;
 if (!f.coilPulseLimiter.asserted)r.flags|=LIMITER_FAULT;
 if (!f.coilCurrentZero.asserted)r.flags|=COIL_ZERO_FAULT;
 if (!f.motorCurrentZero.asserted)r.flags|=MOTOR_ZERO_FAULT;
 // Fixed, auditable prioritization: report all faults in flags while reason
 // conveys the highest-priority fault for a one-line diagnostic.
 if(r.flags&STOP_FAULT)r.reason=Fault::STOP_OPEN;
 else if(r.flags&WATCHDOG_FAULT)r.reason=Fault::WATCHDOG_LOST;
 else if(r.flags&DISCONNECT_FAULT)r.reason=Fault::DISCONNECT_OPEN;
 else if(r.flags&LIMITER_FAULT)r.reason=Fault::LIMITER_NOT_READY;
 else if(r.flags&COIL_ZERO_FAULT)r.reason=Fault::COIL_CURRENT_NONZERO;
 else if(r.flags&MOTOR_ZERO_FAULT)r.reason=Fault::MOTOR_CURRENT_NONZERO;
 else r.reason=Fault::UNCOMMISSIONED;
 r.safeToActuate=false;return r;
}
// Binary LOCAL diagnostic sidecar only. NOT a RadioPayloadKind, command or
// authenticated RF payload. No remote decoder exists until protocol review.
// [magic 0xD5, version 1, reason, flags LE16, millivolts LE16, sequence]
inline void encodeLocal(const Status &s,uint8_t out[8]){
 if(!out)return;
 out[0]=0xD5;out[1]=1U;out[2]=static_cast<uint8_t>(s.reason);
 out[3]=(uint8_t)s.flags;out[4]=(uint8_t)(s.flags>>8);
 out[5]=(uint8_t)s.millivolts;out[6]=(uint8_t)(s.millivolts>>8);
 out[7]=s.sequence;
}
// Tracks changes in diagnostic condition; no output/rearm side effects.
class ChangeTracker {
public:
 bool observe(const Status &s) {
  const bool changed=!hasPrevious_ || prior_.reason!=s.reason ||
                     prior_.flags!=s.flags;
  prior_=s;hasPrevious_=true;
  if(changed)++events_;
  return changed;
 }
 uint32_t events()const{return events_;}
 void reset(){prior_=Status{};hasPrevious_=false;events_=0;}
private:
 Status prior_{};
 bool hasPrevious_=false;
 uint32_t events_=0;
};
} // namespace boat_r30e
