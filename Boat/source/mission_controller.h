#pragma once
#include "mission_control_wire.h"
#include "mission_execution.h"
// Application-level simulation supervisor. Does not actuate GPIO, authorize
// motor commands or claim physical mode changes. A trusted application must
// supply every safety input; default/false readiness inhibits all starts.
class BoatMissionController {
public:
 void resetSession(){engine_=MissionExecution{};plan_={};phase_=MC_IDLE;stagedAt_=0;lastRequest_=0;lastOp_=0;lastResult_=MC_STATUS;diverted_=false;}
 bool stage(const MissionPlan &p,uint32_t now,bool authenticated,bool stationaryStopVerified){
  if(!authenticated||!stationaryStopVerified||!missionPlan_valid(p)||
     (phase_!=MC_IDLE&&phase_!=MC_PREPARED&&phase_!=MC_COMPLETE&&phase_!=MC_ABORTED&&phase_!=MC_MANUAL_REQUESTED)||
     (plan_.transaction&&p.transaction<=plan_.transaction))return false;
  diverted_=false;plan_=p;phase_=MC_PREPARED;stagedAt_=now;lastRequest_=0;lastOp_=0;lastResult_=MC_STATUS;return true;
 }
 void emergencyAbort(){if(plan_.transaction){engine_.abort();phase_=MC_ABORTED;if(lastOp_!=MCTRL_CANCEL)lastResult_=MC_REJECTED;}}
 // Local fail-safe authorization belongs to the previously started mission,
 // not to a new radio session. No radio request can resume its old target.
 bool divertHome(const ExecInputs &in,bool appReady){
  if(!appReady||!engine_.divertHome(in)){emergencyAbort();return false;}
  diverted_=true;phase_=MC_RETURNING;lastResult_=MC_REJECTED;return true;
 }
 bool diverted()const{return diverted_&&phase_==MC_RETURNING;}
 MissionNavigationResult navigation(const ExecInputs &in)const{return engine_.navigation(in);}
 bool confirmManual(const ExecInputs &in){if(phase_!=MC_MANUAL_REQUESTED||!in.stationary||!in.releaseIdle||in.stopActive)return false;phase_=MC_IDLE;return true;}
 uint16_t lastRequestId()const{return lastRequest_;}
 MissionControlFeedback status()const{return {plan_.transaction,phase_,0,0,MC_STATUS};}
 bool request(const MissionControlRequest &r,const ExecInputs &in,uint32_t now,bool authenticated,bool appReady,bool sticksNeutral,MissionControlFeedback &ack){
  if(!authenticated||!plan_.transaction||r.transaction!=plan_.transaction||!r.request||r.op<MCTRL_START||r.op>MCTRL_SWITCH_MANUAL)return false;
  if(r.request<lastRequest_)return false;
  if(r.request==lastRequest_){
   if((uint8_t)r.op!=lastOp_)return false;
   ack={r.transaction,phase_,lastOp_,lastRequest_,lastResult_};return true;
  }
  bool accepted=false;
  if(r.op==MCTRL_CANCEL){engine_.abort();phase_=MC_ABORTED;accepted=true;}
  else if(appReady&&in.primaryFresh&&!in.stopActive){
   switch(r.op){
   case MCTRL_START:
    if(phase_==MC_PREPARED&&now-stagedAt_<10000U&&sticksNeutral&&engine_.start(plan_,in,now,true)){phase_=MC_OUTBOUND;accepted=true;}
    break;
   case MCTRL_PAUSE:
    if((phase_==MC_OUTBOUND||phase_==MC_RETURNING)&&engine_.pause(true)){phase_=MC_PAUSED;accepted=true;}
    break;
   case MCTRL_TEMP_OVERRIDE:
    if((phase_==MC_OUTBOUND||phase_==MC_RETURNING)&&engine_.pause(true)){phase_=MC_OVERRIDE_NEUTRAL;accepted=true;}
    break;
   case MCTRL_RESUME:
    if((phase_==MC_PAUSED||phase_==MC_OVERRIDE_ACTIVE||phase_==MC_OVERRIDE_NEUTRAL)&&sticksNeutral&&engine_.resume(in,true)){
     phase_=engine_.phase()==EXEC_RETURNING?MC_RETURNING:MC_OUTBOUND;accepted=true;
    }
    break;
   case MCTRL_SWITCH_MANUAL:{
    const bool safeRthTakeover=!diverted_||
      (sticksNeutral&&in.stationary&&in.releaseIdle&&in.releaseInterlocks);
    if(safeRthTakeover&&
       (phase_==MC_OUTBOUND||phase_==MC_RETURNING||phase_==MC_PAUSED||phase_==MC_OVERRIDE_NEUTRAL||phase_==MC_OVERRIDE_ACTIVE)){
     engine_.abort();phase_=MC_MANUAL_REQUESTED;accepted=true;diverted_=false;
    }
    break;
   }
   default:break;
   }
  }
  lastRequest_=r.request;lastOp_=(uint8_t)r.op;lastResult_=accepted?MC_ACCEPTED:MC_REJECTED;
  ack={r.transaction,phase_,lastOp_,lastRequest_,lastResult_};return true;
 }
 ExecOutput tick(const ExecInputs &in,uint32_t now,bool appReady,bool sticksNeutral){
  const ExecOutput stop={INTENT_STOP,-1,0};
  if(phase_==MC_IDLE||phase_==MC_COMPLETE||phase_==MC_ABORTED||phase_==MC_MANUAL_REQUESTED)return stop;
  if(!appReady||in.stopActive){
   // PREPARED is intentionally permitted to wait while physically STOPped.
   if(phase_!=MC_PREPARED||!appReady||now-stagedAt_>=10000U){engine_.abort();phase_=MC_ABORTED;}
   return stop;
  }
  if(phase_==MC_PREPARED){if(now-stagedAt_>=10000U){phase_=MC_ABORTED;}return stop;}
  // Once a release intent has entered its execution window, primary-link loss
  // makes completion ambiguous. Abort it even if both radios disappear; never
  // replay a hopper/hook release after reconnect.
  if(engine_.phase()==EXEC_RELEASE_WAIT&&!in.primaryFresh){engine_.abort();phase_=MC_ABORTED;return stop;}
  // Both radio paths absent: STOP and preserve the mission phase. The sole
  // exception is a local, NVS-validated Home return after the 10-second gate.
  if(!in.primaryFresh&&!in.backupFresh&&!in.autonomousRth)return stop;
  auto out=engine_.tick(in,now);
  switch(engine_.phase()){
   case EXEC_OUTBOUND:phase_=MC_OUTBOUND;break;
   case EXEC_AT_TARGET:case EXEC_RELEASE_WAIT:phase_=MC_RELEASING;break;
   case EXEC_RETURNING:phase_=MC_RETURNING;break;
   case EXEC_PAUSED:
    if(phase_==MC_OVERRIDE_NEUTRAL&&sticksNeutral&&in.stationary&&in.releaseIdle)phase_=MC_OVERRIDE_ACTIVE;
    break;
   case EXEC_COMPLETE:phase_=MC_COMPLETE;break;
   case EXEC_ABORTED:phase_=MC_ABORTED;break;
   default:break;
  }
  return out;
 }
 bool releaseComplete(uint16_t tx,uint16_t token,bool success,bool verifiedIdle){return engine_.releaseComplete(tx,token,success,verifiedIdle);}
 // A status hint only. No manual motor output is generated here; the future
 // executor must validate and lease each fresh authenticated stick command.
 bool temporaryOverrideConfirmed()const{return phase_==MC_OVERRIDE_ACTIVE;}
 bool canDivertHome()const{
  return phase_==MC_OUTBOUND||phase_==MC_PAUSED||phase_==MC_RETURNING||
         phase_==MC_OVERRIDE_NEUTRAL||phase_==MC_OVERRIDE_ACTIVE||
         (phase_==MC_RELEASING&&engine_.phase()==EXEC_AT_TARGET);
 }
 bool homeNavigationValid(const ExecInputs &in)const{return plan_.transaction&&engine_.homeNavigationValid(in);}
 bool homeMatches(int32_t latE7,int32_t lonE7)const{
  return plan_.transaction&&plan_.homeLatE7==latE7&&plan_.homeLonE7==lonE7;
 }
 bool cancelOverrideForLinkLoss(){
  if(phase_!=MC_OVERRIDE_ACTIVE&&phase_!=MC_OVERRIDE_NEUTRAL)return false;
  if(!engine_.cancelPauseForLinkLoss()){emergencyAbort();return false;}
  phase_=engine_.phase()==EXEC_RETURNING?MC_RETURNING:MC_OUTBOUND;
  lastResult_=MC_REJECTED;
  return true;
 }
private:
 bool diverted_=false;
 MissionExecution engine_;MissionPlan plan_{};MissionControlPhase phase_=MC_IDLE;uint32_t stagedAt_=0;
 uint16_t lastRequest_=0;uint8_t lastOp_=0;MissionControlResult lastResult_=MC_STATUS;
};
