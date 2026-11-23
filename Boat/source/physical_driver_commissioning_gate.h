// LEGACY, RETIRED: dedicated independent hardware safety architecture is
// explicitly out of scope as of 2026-11-23. Do not integrate this historical
// contract into the active software-only Boat driver. Retained solely for
// compatibility with archived tests; it does not authorize physical outputs.
#pragma once
#include <stdint.h>
#include "boat_hardware_supervisor.h"
// HISTORICAL R48 LEGACY feedback schema: requires distinct motor and
// coil contactor status. The R29/R30C-selected Boat uses ONE shared 3S
// actuator power bus and has NO separately-switched motor/coil domains.
// Do NOT short, duplicate, alias or fake these two legacy contactor fields
// to claim independent safety. See boat_common_bus_feedback_r30c.h for the
// proposed ONE-common-bus supervision contract (still uncommissioned).
// "Independent" means functionally capable of cutting/observing load power
// despite a stalled MCU, NOT a second actuator battery.
// Fail closed when any real local feedback is missing or stale.
struct BoatSafetyContacts {
 bool stopLoopClosed=false;
 bool watchdogContactClosed=false;
 bool motorContactorFeedbackClosed=false;
 bool coilContactorFeedbackClosed=false;
 bool coilCurrentZero=false;
 bool motorCurrentZero=false;
 bool batteryHealthy=false; // Derived from fresh locally commissioned INA231 BUS voltage (R27/R30B), not a separate contact.
 bool isolationTestSigned=false;
 uint32_t sampledAt=0;
};
inline bool boatPhysicalOutputsPermitted(const BoatSafetyContacts &s,uint32_t now){
 return s.isolationTestSigned&&s.stopLoopClosed&&s.watchdogContactClosed&&
 s.motorContactorFeedbackClosed&&s.coilContactorFeedbackClosed&&
 s.coilCurrentZero&&s.batteryHealthy&&
 (uint32_t)(now-s.sampledAt)<100U;
}
inline bool boatPhysicalCoilPulsePermitted(const BoatSafetyContacts &s,uint32_t now,
                                            int8_t channel,uint16_t pulseMs,
                                            bool otherChannelsInactive){
 return boatPhysicalOutputsPermitted(s,now)&&s.motorCurrentZero&&
 otherChannelsInactive&&channel>=0&&channel<4&&pulseMs>=1&&pulseMs<=500;
}
inline void boatPhysicalSafetyToSample(const BoatSafetyContacts &c,uint32_t now,
                                       BoatHardwareSample &s){
 const bool fresh=(uint32_t)(now-c.sampledAt)<100U;
 s.safetyAt=c.sampledAt;s.safetyValid=fresh&&c.isolationTestSigned;
 s.physicalStopReleased=fresh&&c.stopLoopClosed;
 s.watchdogReady=fresh&&c.watchdogContactClosed;
 s.outputFeedbackValid=fresh&&c.motorContactorFeedbackClosed&&c.coilContactorFeedbackClosed;
 s.releaseIdle=fresh&&c.coilCurrentZero;
 s.releaseInterlocks=fresh&&c.motorCurrentZero&&c.coilCurrentZero;
 s.batteryHealthy=fresh&&c.batteryHealthy;
}
