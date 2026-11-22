#pragma once
#include "mission_navigation.h"
// Pure execution supervisor. Outputs are requests only, never GPIO/PWM.
// The future Boat application must enforce a separate hardware watchdog,
// motor/solenoid interlocks, and authenticated action/feedback gates.
enum ExecPhase:uint8_t{EXEC_IDLE,EXEC_OUTBOUND,EXEC_AT_TARGET,EXEC_RELEASE_WAIT,EXEC_RETURNING,EXEC_PAUSED,EXEC_COMPLETE,EXEC_ABORTED};
enum ExecIntent:uint8_t{INTENT_STOP,INTENT_NAV_TARGET,INTENT_NAV_HOME,INTENT_RELEASE};
struct ExecInputs{
 MissionNavigationFix fix;
 bool stopActive,primaryFresh,backupFresh,stationary,releaseInterlocks,releaseIdle;
};
struct ExecOutput{ExecIntent intent;int8_t channel;uint16_t token;};
class MissionExecution{
public:
 bool start(const MissionPlan &p,const ExecInputs &in,uint32_t now,bool authenticated){
  if(!authenticated||!in.primaryFresh||in.stopActive||!in.stationary||!in.releaseIdle||
     !missionNavigation_evaluate(p,in.fix,false).valid||
     (phase_!=EXEC_IDLE&&phase_!=EXEC_COMPLETE&&phase_!=EXEC_ABORTED)||
     !p.transaction||p.transaction==lastTransaction_)return false;
  plan_=p;lastTransaction_=p.transaction;remaining_=p.releaseMask;phase_=EXEC_OUTBOUND;
  releaseToken_=0;releaseAt_=now;return true;
 }
 void abort(){phase_=EXEC_ABORTED;remaining_=0;}
 // Irreversible diversion: discard all release work. Never travel with an
 // unconfirmed release pulse, invalid Home navigation or active STOP.
 bool divertHome(const ExecInputs &in){
  if(phase_!=EXEC_OUTBOUND&&phase_!=EXEC_AT_TARGET&&phase_!=EXEC_RETURNING&&phase_!=EXEC_PAUSED)return false;
  if(in.stopActive||!in.releaseIdle||!missionNavigation_evaluate(plan_,in.fix,true).valid){abort();return false;}
  remaining_=0;phase_=EXEC_RETURNING;return true;
 }
 MissionNavigationResult navigation(const ExecInputs &in)const{return missionNavigation_evaluate(plan_,in.fix,phase_==EXEC_RETURNING);}
 bool pause(bool authenticated){
  if(!authenticated||(phase_!=EXEC_OUTBOUND&&phase_!=EXEC_RETURNING))return false;
  resumePhase_=phase_;phase_=EXEC_PAUSED;return true;
 }
 bool resume(const ExecInputs &in,bool authenticated){
  if(!authenticated||phase_!=EXEC_PAUSED||!in.primaryFresh||in.stopActive||!in.stationary||
     !in.releaseIdle||!missionNavigation_evaluate(plan_,in.fix,resumePhase_==EXEC_RETURNING).valid)return false;
  phase_=resumePhase_;return true;
 }
 bool releaseComplete(uint16_t transaction,uint16_t token,bool success,bool verifiedIdle){
  if(phase_!=EXEC_RELEASE_WAIT||transaction!=plan_.transaction||token!=releaseToken_)return false;
  if(!success||!verifiedIdle){abort();return true;}
  phase_=EXEC_AT_TARGET;return true;
 }
 ExecOutput tick(const ExecInputs &in,uint32_t now){
  const ExecOutput stopped={INTENT_STOP,-1,0};
  if(phase_==EXEC_IDLE||phase_==EXEC_COMPLETE||phase_==EXEC_ABORTED)return stopped;
  if(in.stopActive){abort();return stopped;}
  const bool navHome=phase_==EXEC_RETURNING||(phase_==EXEC_PAUSED&&resumePhase_==EXEC_RETURNING);
  const auto nav=missionNavigation_evaluate(plan_,in.fix,navHome);
  if(!nav.valid){abort();return stopped;}
  // Loss of primary prevents new releases. If already pulsing, stop and
  // abort rather than move with an unconfirmed actuator.
  if(phase_==EXEC_RELEASE_WAIT){
   if(!in.primaryFresh||!in.releaseInterlocks||!nav.arrived||now-releaseAt_>=5000U)abort();
   return stopped;
  }
  // A 3-second LoRa slot is not equivalent to fresh primary control.
  if(!in.primaryFresh){
   if(!divertHome(in)){abort();return stopped;}
   if(!navHome)return stopped; // recompute Home geometry next tick
  }
  if(phase_==EXEC_PAUSED)return stopped;
  if(!in.releaseIdle){abort();return stopped;}
  if(phase_==EXEC_OUTBOUND){
   if(nav.arrived){phase_=EXEC_AT_TARGET;return stopped;}
   return {INTENT_NAV_TARGET,-1,0};
  }
  if(phase_==EXEC_AT_TARGET){
   if(!in.stationary)return stopped;
   if(!remaining_||!in.primaryFresh){remaining_=0;phase_=EXEC_RETURNING;return stopped;}
   if(!nav.arrived||!in.releaseInterlocks){abort();return stopped;}
   for(uint8_t i=0;i<4;++i)if(remaining_&(1U<<i)){
    remaining_&=(uint8_t)~(1U<<i);++releaseToken_;releaseAt_=now;phase_=EXEC_RELEASE_WAIT;
    return {INTENT_RELEASE,(int8_t)i,releaseToken_};
   }
   return stopped;
  }
  if(phase_==EXEC_RETURNING){
   if(nav.arrived){if(in.stationary)phase_=EXEC_COMPLETE;return stopped;}
   return {INTENT_NAV_HOME,-1,0};
  }
  abort();return stopped;
 }
 ExecPhase phase()const{return phase_;}
private:
 MissionPlan plan_{};ExecPhase phase_=EXEC_IDLE,resumePhase_=EXEC_IDLE;
 uint8_t remaining_=0;uint16_t releaseToken_=0,lastTransaction_=0;uint32_t releaseAt_=0;
};
