#pragma once
#include <stdint.h>
#include "figure8_auto_session.h"
// Produces differential-thrust INTENT only. No GPIO, motors, or solenoids.
// Physical driver and safety supervisor MUST independently authorize actuation.
struct Figure8MotionSafety {
 bool authenticatedRequest=false,primaryFresh=false,physicalStopReleased=false;
 bool watchdogReady=false,batteryHealthy=false,geofenceOK=false;
 bool gnssFresh=false,stationaryAtStart=false,releaseIdle=false;
 bool independentMotorInterlock=false,manualOverride=false;
};
struct Figure8MotorIntent{int16_t left=0,right=0;bool valid=false;};
enum class Figure8MotionState:uint8_t{IDLE,RUNNING,COMPLETE,FAILED,ABORTED};
class Figure8AutonomousMotion {
public:
 bool start(uint32_t now,const Figure8MotionSafety &s){
  if(state_==Figure8MotionState::RUNNING||!canStart(s))return false;
  start_=now;last_=now;state_=Figure8MotionState::RUNNING;return true;
 }
 Figure8MotorIntent tick(uint32_t now,const Figure8MotionSafety &s,Figure8AutoState calibration){
  Figure8MotorIntent stop{};
  if(state_!=Figure8MotionState::RUNNING)return stop;
  if(!canContinue(s)||calibration==Figure8AutoState::TIMED_OUT||calibration==Figure8AutoState::ABORTED||
     (uint32_t)(now-last_)>250U||(uint32_t)(now-start_)>180000U){
   state_=Figure8MotionState::FAILED;return stop;
  }
  last_=now;
  if(calibration==Figure8AutoState::CANDIDATE_READY){
   state_=Figure8MotionState::COMPLETE;return stop;
  }
  // Two opposing semicircles, repeated. Nominal bounded thrust requests only.
  // Physical PWM/current/speed limits are imposed by the separate actuator driver.
  const uint32_t segment=((uint32_t)(now-start_)/12000U)%2U;
  return segment==0?Figure8MotorIntent{180,100,true}:Figure8MotorIntent{100,180,true};
 }
 void abort(){state_=Figure8MotionState::ABORTED;}
 Figure8MotionState state()const{return state_;}
private:
 static bool canStart(const Figure8MotionSafety &s){
  return canContinue(s)&&s.authenticatedRequest&&s.stationaryAtStart;
 }
 static bool canContinue(const Figure8MotionSafety &s){
  return s.primaryFresh&&s.physicalStopReleased&&s.watchdogReady&&s.batteryHealthy&&
   s.geofenceOK&&s.gnssFresh&&s.releaseIdle&&s.independentMotorInterlock&&!s.manualOverride;
 }
 Figure8MotionState state_=Figure8MotionState::IDLE;
 uint32_t start_=0,last_=0;
};
