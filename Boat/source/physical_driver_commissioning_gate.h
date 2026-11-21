#pragma once
#include <stdint.h>
#include "boat_hardware_supervisor.h"
// R48 independent-contact feedback adapter. The inputs MUST come from local
// isolated, independently powered hardware, not remote radio messages.
// Fail closed when any feedback is missing or stale.
struct BoatSafetyContacts {
 bool stopLoopClosed=false;
 bool watchdogContactClosed=false;
 bool motorContactorFeedbackClosed=false;
 bool coilContactorFeedbackClosed=false;
 bool coilCurrentZero=false;
 bool motorCurrentZero=false;
 bool batteryHealthy=false;
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
