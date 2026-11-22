#pragma once
#include "mission_execution.h"
#include "boat_battery_policy.h"
#include <cmath>
// Hardware measurements must be acquired locally. Network messages cannot
// supply these timestamps or override the physical STOP/interlock state.
struct BoatHardwareSample {
 MissionNavigationFix fix;
 uint32_t safetyAt,gnssAt,headingAt;
 bool safetyValid,physicalStopReleased,batteryHealthy,stationary,releaseInterlocks,releaseIdle,watchdogReady;
 bool outputFeedbackValid,leftMotorActive,rightMotorActive;
 uint8_t batteryPercent,gpsFixType,satellites;
 uint16_t batteryMillivolts,speedCmPerSecond;
 uint32_t batteryAt,speedAt;
 bool voltageValid,speedValid;
 BoatBatteryBand batteryBand;
 bool batteryRthRequested;
};
struct BoatActuatorConfig {
 uint16_t pulseMs[4]; // actual coil characterization required
 uint16_t maxNavMotor; // bench-tuned demand cap, not a PWM pin assignment
 double yawGain;
};
struct BoatActuatorOutput{int16_t left,right;int8_t releaseChannel;};
inline ExecInputs boatHardware_inputs(const BoatHardwareSample &s,uint32_t now,bool stopLatched){
 const bool safe=s.safetyValid&&now-s.safetyAt<100U&&s.watchdogReady&&s.outputFeedbackValid;
 auto fix=s.fix;
 fix.fresh=fix.fresh&&now-s.gnssAt<2000U;
 fix.headingValid=fix.headingValid&&now-s.headingAt<300U;
 fix.batteryHealthy=safe&&s.batteryHealthy;
 const bool batteryLow=s.batteryBand>=BOAT_BATTERY_RED;
 return {fix,stopLatched||!safe||!s.physicalStopReleased,false,false,safe&&s.stationary&&!s.leftMotorActive&&!s.rightMotorActive,safe&&s.releaseInterlocks,safe&&s.releaseIdle,false,batteryLow};
}
inline bool boatHardware_configValid(const BoatActuatorConfig &c){
 if(!c.maxNavMotor||c.maxNavMotor>1000||!std::isfinite(c.yawGain)||c.yawGain<=0||c.yawGain>1000)return false;
 for(auto ms:c.pulseMs)if(!ms||ms>=4000)return false;
 return true;
}
// Actuator-side software watchdog. A separate hardware watchdog/current
// cutoff is mandatory; this class cannot deenergize a stuck MCU by itself.
class BoatActuatorSupervisor{
public:
 bool configure(const BoatActuatorConfig &cfg){stop();cfg_=cfg;configured_=boatHardware_configValid(cfg);return configured_;}
 void stop(){stopLatched_=true;manualValid_=false;pulseActive_=false;awaitIdle_=false;releaseCancelled_=false;completion_={};}
 bool clearStop(const BoatHardwareSample &s,uint32_t now,bool authenticatedPrimary,bool neutral){
  const auto in=boatHardware_inputs(s,now,false);
  if(!configured_||!authenticatedPrimary||!neutral||in.stopActive||!in.fix.batteryHealthy||!in.stationary||!in.releaseIdle||!in.releaseInterlocks)return false;
  stopLatched_=false;return true;
 }
 bool stopped()const{return stopLatched_;}
 void manual(int16_t left,int16_t right,uint32_t receivedAt,bool authenticatedPrimary){
  if(manualNeedsNeutral_){if(authenticatedPrimary&&!pulseActive_&&!awaitIdle_&&!left&&!right)manualNeedsNeutral_=false;manualValid_=false;return;}
  manualValid_=!pulseActive_&&!awaitIdle_&&authenticatedPrimary&&!stopLatched_&&left>=-1000&&left<=1000&&right>=-1000&&right<=1000;
  manualLeft_=left;manualRight_=right;manualAt_=receivedAt;
 }
 void cancelManual(){manualValid_=false;}
 bool releaseBusy()const{return pulseActive_||awaitIdle_;}
 void cancelReleaseForFailsafe(uint32_t now){if(pulseActive_){pulseActive_=false;awaitIdle_=true;offAt_=now;releaseCancelled_=true;cancelManual();}}
 bool beginManualRelease(uint16_t id,int8_t channel,const ExecInputs &in,uint32_t now){
  if(releaseBusy()||(manualValid_&&now-manualAt_<200U&&(manualLeft_||manualRight_)))return false;
  newMissionIdentity();if(!beginRelease(id,1,channel,in,now))return false;
  manualRelease_=true;manualNeedsNeutral_=true;cancelManual();return true;
 }
 // Mission release is a single tokenized edge, not a continuously repeated command.
 bool beginRelease(uint16_t tx,uint16_t token,int8_t channel,const ExecInputs &in,uint32_t now){
  if(!configured_||stopLatched_||!tx||!token||channel<0||channel>3||pulseActive_||awaitIdle_||
    (tx==lastTx_&&token<=lastToken_)||in.stopActive||!in.primaryFresh||!in.stationary||
    !in.releaseIdle||!in.releaseInterlocks||!in.fix.batteryHealthy||in.batteryLow)return false;
  manualRelease_=false;releaseCancelled_=false;lastTx_=tx;lastToken_=token;channel_=channel;pulseAt_=now;pulseActive_=true;return true;
 }
 void newMissionIdentity(){if(!pulseActive_&&!awaitIdle_){lastTx_=lastToken_=0;}}
 BoatActuatorOutput tick(const ExecInputs &in,uint32_t now,ExecOutput intent,const MissionNavigationResult &nav,bool missionActive,bool overrideActive,int16_t overrideLeft,int16_t overrideRight,uint32_t safetySampleAt){
  const BoatActuatorOutput off={0,0,-1};
  if(!configured_||stopLatched_||in.stopActive||!in.fix.batteryHealthy){stop();return off;}
  if(in.batteryLow&&missionActive&&intent.intent!=INTENT_NAV_HOME){
   // During a battery abort, deenergize any ongoing release *immediately*.
   // Still advance the post-off feedback gate, otherwise awaitIdle_ would
   // remain permanently set and safe Home diversion could never begin.
   if(pulseActive_)cancelReleaseForFailsafe(now);
   if(awaitIdle_){
    if((int32_t)(safetySampleAt-offAt_)>0&&in.releaseIdle){
     completion_={lastTx_,lastToken_,false,true};
     awaitIdle_=false;releaseCancelled_=false;
    }else if((uint32_t)(now-offAt_)>=500U){
     completion_={lastTx_,lastToken_,false,true};
     awaitIdle_=false;releaseCancelled_=false;stopLatched_=true;
    }
   }
   return off;
  }
  // Never retain/replay a manual stick command after primary-link loss.
  if(!in.primaryFresh){cancelManual();manualNeedsNeutral_=true;}
  // Both authenticated radio paths stale: all outputs OFF, except a locally
  // authorized INTENT_NAV_HOME after the 10-second gate. Releases stay OFF.
  // An interrupted release is latched because its physical completion is unknown.
  const bool radioIndependentHome=in.autonomousRth&&missionActive&&intent.intent==INTENT_NAV_HOME;
  if(!in.primaryFresh&&!in.backupFresh&&!radioIndependentHome){
   if(releaseBusy()){
    completion_={lastTx_,lastToken_,false,true};
    pulseActive_=awaitIdle_=false;
    stopLatched_=true;
   }
   return off;
  }
  if(pulseActive_||awaitIdle_){
   cancelManual();
   if((manualRelease_?missionActive:!missionActive)||!in.primaryFresh||!in.stationary||!in.releaseInterlocks){
    completion_={lastTx_,lastToken_,false,true};pulseActive_=awaitIdle_=false;stopLatched_=true;return off;
   }
   if(pulseActive_){
    if(now-pulseAt_<cfg_.pulseMs[channel_])return {0,0,channel_};
    pulseActive_=false;awaitIdle_=true;offAt_=now;releaseCancelled_=false;return off;
   }
   // Require a separate post-deenergization idle sample, not same-tick confirmation.
   if((int32_t)(safetySampleAt-offAt_)>0&&in.releaseIdle){completion_={lastTx_,lastToken_,!releaseCancelled_,true};awaitIdle_=false;releaseCancelled_=false;}
   else if(now-offAt_>=500U){completion_={lastTx_,lastToken_,false,true};awaitIdle_=false;releaseCancelled_=false;stopLatched_=true;}
   return off;
  }
  if(!in.releaseIdle)return off;
  if(missionActive){
   cancelManual();
   if(overrideActive&&in.primaryFresh&&in.releaseInterlocks&&overrideLeft>=-1000&&overrideLeft<=1000&&overrideRight>=-1000&&overrideRight<=1000)
    return {overrideLeft,overrideRight,-1};
   if(intent.intent!=INTENT_NAV_TARGET&&intent.intent!=INTENT_NAV_HOME)return off;
   if(!nav.valid||nav.arrived||!std::isfinite(nav.headingErrorDeg)||!std::isfinite(nav.distanceM)||!in.releaseInterlocks)return off;
   // Differential thrust ONLY: positive bearing error means clockwise/right,
   // hence the port/left propeller gets more forward thrust than starboard.
   // IMPORTANT: uncommissioned autonomous pivot/reverse is disallowed.
   // Manual TANK mode retains signed reverse commands under its own safety lease.
   const double cap=cfg_.maxNavMotor;
   const double turn=std::fmax(-cap,std::fmin(cap,nav.headingErrorDeg*cfg_.yawGain));
   const double forward=std::fabs(nav.headingErrorDeg)>45.0?0.0:cap*(1.0-std::fabs(nav.headingErrorDeg)/90.0);
   const double left=std::fmax(0.0,std::fmin(cap,forward+turn));
   const double right=std::fmax(0.0,std::fmin(cap,forward-turn));
   return {(int16_t)left,(int16_t)right,-1};
  }
  if(manualValid_&&in.primaryFresh&&now-manualAt_<200U&&in.releaseInterlocks)return {manualLeft_,manualRight_,-1};
  cancelManual();return off;
 }
 struct Completion{uint16_t transaction,token;bool success,valid;};
 Completion takeCompletion(){auto value=completion_;completion_={};return value;}
private:
 BoatActuatorConfig cfg_{};bool manualNeedsNeutral_=false,manualRelease_=false,configured_=false,stopLatched_=true,manualValid_=false,pulseActive_=false,awaitIdle_=false,releaseCancelled_=false;
 int16_t manualLeft_=0,manualRight_=0;int8_t channel_=-1;
 uint32_t manualAt_=0,pulseAt_=0,offAt_=0;
 uint16_t lastTx_=0,lastToken_=0;Completion completion_{};
};
