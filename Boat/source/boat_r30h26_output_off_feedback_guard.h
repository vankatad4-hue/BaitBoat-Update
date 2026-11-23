#pragma once
#include "boat_hardware_supervisor.h"
#include <stdint.h>

// R30H-26: SOFTWARE-ONLY mismatch monitor for locally reported motor/coil
// output state after the physical driver was commanded OFF.
// Feedback names are not independently calibrated current measurements.
// The commercial hardware watchdog/current cutoff remains mandatory.
namespace boat_r30h26 {
class OutputOffFeedbackGuard {
public:
 static constexpr uint32_t MOTOR_OFF_SETTLE_MS=250U;
 static constexpr uint32_t RELEASE_OFF_SETTLE_MS=500U;

 // Called after a physical disable() request (attach, STOP, brownout).
 // Never assume disable() succeeded: it merely starts the feedback clock.
 void commandedDisable(uint32_t now){
  motorsOffCommanded_=releaseOffCommanded_=true;
  motorsOffSince_=releaseOffSince_=now;
 }
 // Called ONLY after driver.apply() accepts the command. Repeated OFF
 // requests do not move the deadline; an actual energizing demand does.
 void acceptedOutput(const BoatActuatorOutput &requested,uint32_t now){
  const bool motorsOff=requested.left==0&&requested.right==0;
  const bool releaseOff=requested.releaseChannel<0;
  if(motorsOff){
   if(!motorsOffCommanded_){motorsOffCommanded_=true;motorsOffSince_=now;}
  }else motorsOffCommanded_=false;
  if(releaseOff){
   if(!releaseOffCommanded_){releaseOffCommanded_=true;releaseOffSince_=now;}
  }else releaseOffCommanded_=false;
 }
 bool motorStuck(const BoatHardwareSample &sample,uint32_t now) const {
  return feedbackFresh(sample,now)&&motorsOffCommanded_&&
    (uint32_t)(now-motorsOffSince_)>=MOTOR_OFF_SETTLE_MS&&
    (sample.leftMotorActive||sample.rightMotorActive);
 }
 bool coilStuck(const BoatHardwareSample &sample,uint32_t now) const {
  return feedbackFresh(sample,now)&&releaseOffCommanded_&&
    (uint32_t)(now-releaseOffSince_)>=RELEASE_OFF_SETTLE_MS&&
    !sample.releaseIdle;
 }
 bool mismatch(const BoatHardwareSample &sample,uint32_t now) const {
  return motorStuck(sample,now)||coilStuck(sample,now);
 }
private:
 static bool feedbackFresh(const BoatHardwareSample &s,uint32_t now){
  return s.safetyValid&&s.outputFeedbackValid&&s.watchdogReady&&
         (uint32_t)(now-s.safetyAt)<100U;
 }
 bool motorsOffCommanded_=false,releaseOffCommanded_=false;
 uint32_t motorsOffSince_=0,releaseOffSince_=0;
};
} // namespace boat_r30h26
