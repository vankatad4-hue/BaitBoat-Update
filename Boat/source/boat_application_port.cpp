#include "boat_application_port.h"
#include "boat_hardware_status.h"
#include "boat_hardware_telemetry.h"
BoatApplicationPort *BoatApplicationPort::active_=nullptr;
bool BoatApplicationPort::attach(const BoatPhysicalDriver &driver,const BoatActuatorConfig &config){
 if(attached_||active_||!driver.read||!driver.apply||!driver.disable)return false;
 driver.disable();if(!executor_.configure(config))return false;
 driver_=driver;attached_=true;active_=this;
 radio_.setMissionInputs(readMission);radio_.setMissionPreparationGate(prepareGate);
 return true;
}
bool BoatApplicationPort::readMission(ExecInputs &in){
 in={};if(!active_||!active_->attached_||!active_->sampleOK_)return false;
 in=boatHardware_inputs(active_->sample_,millis(),active_->executor_.stopped());return true;
}
bool BoatApplicationPort::prepareGate(){
 if(!active_||!active_->sampleOK_)return false;
 const auto in=boatHardware_inputs(active_->sample_,millis(),false);
 return active_->executor_.stopped()&&!in.stopActive&&in.stationary&&in.releaseIdle&&in.releaseInterlocks&&in.fix.batteryHealthy;
}
void BoatApplicationPort::finishManual(bool success){
 if(!manualPending_)return;
 manualPending_=false;
 const auto status=boatHardware_status(sample_,millis(),executor_.stopped(),radio_.missionPhase(),radio_.detachedReturnActive(),radio_.primaryFresh(millis()));
 radio_.completeApplicationAction(manualRelease_,generation_,success,status.stopState,(status.safetyFlags&1)?1:0);
}
void BoatApplicationPort::publishData(uint32_t now){
 const auto b=boatHardware_telemetry(sample_,now,++sampleSequence_,255,radio_.missionPhase());
 for(uint8_t i=0;i<b.count;++i)radio_.sendHardwareTelemetry(b.payload[i]);
}
void BoatApplicationPort::fault(){
 finishManual(false);
 executor_.stop();radio_.localEmergencyStop();releaseTransaction_=0;
 if(driver_.disable)driver_.disable();
}
void BoatApplicationPort::tick(){
 // Diagnostic default: poll and discard, never simulate a physical input.
 if(!attached_){
  radio_.update();BoatCommand cmd{};bool duplicate=false;ManagedLink link=LINK_NONE;
  while(radio_.takeCommand(cmd,duplicate,link)){}
  return;
 }
 sample_={};sampleOK_=driver_.read(sample_);
 if(!sampleOK_){fault();radio_.update();radio_.sendHardwareTelemetry(boatHardware_status({},millis(),true,MC_ABORTED,false,false));return;}
 const uint32_t now=millis();
 auto in=boatHardware_inputs(sample_,now,executor_.stopped());
 if(!sample_.physicalStopReleased||!sample_.watchdogReady||!in.fix.batteryHealthy){fault();radio_.update();radio_.sendHardwareTelemetry(boatHardware_status(sample_,millis(),true,MC_ABORTED,false,radio_.primaryFresh(millis())));return;}
 radio_.update();
 if(generation_!=radio_.sessionGeneration()){
  if(manualPending_){manualPending_=false;executor_.stop();radio_.localEmergencyStop();driver_.disable();}
  executor_.cancelManual();if(!radio_.detachedReturnActive()){executor_.stop();driver_.disable();}
  generation_=radio_.sessionGeneration();
 }
 in=boatHardware_inputs(sample_,millis(),executor_.stopped());
 in.primaryFresh=radio_.primaryFresh(millis());in.backupFresh=radio_.backupFresh(millis());
 BoatCommand cmd{};bool duplicate=false;ManagedLink link=LINK_NONE;
 while(radio_.takeCommand(cmd,duplicate,link)){
  if(cmd.type==CMD_EMERGENCY_STOP){fault();return;}
  if(duplicate)continue;
  if(cmd.type==CMD_CLEAR_STOP){
   // Remote clear-STOP API checks calibrated neutral sticks; local driver
   // independently confirms stationary propulsion and all interlocks.
   const bool ok=!executor_.releaseBusy()&&executor_.clearStop(sample_,millis(),link==LINK_NRF_PRIMARY&&in.primaryFresh,in.stationary);
   // This ACK confirms the local latch operation only, not physical outputs.
   radio_.completeApplicationAction(cmd,generation_,ok,2,0);
   continue;
  }
  const auto phase=radio_.missionPhase();
  const bool manualMode=phase==MC_IDLE||phase==MC_COMPLETE||phase==MC_ABORTED;
  if(cmd.type==CMD_MANUAL_MOTORS&&manualMode&&!manualPending_)
   executor_.manual(cmd.leftMotor,cmd.rightMotor,cmd.createdAtMs,link==LINK_NRF_PRIMARY&&in.primaryFresh);
  if(cmd.type>=CMD_RELEASE_HOPPER_LEFT&&cmd.type<=CMD_RELEASE_HOOK_RIGHT){
   in=boatHardware_inputs(sample_,millis(),executor_.stopped());in.primaryFresh=radio_.primaryFresh(millis());
   const bool allowed=manualMode&&!manualPending_&&link==LINK_NRF_PRIMARY&&cmd.activate&&
     executor_.beginManualRelease((uint16_t)cmd.commandId,(int8_t)(cmd.type-CMD_RELEASE_HOPPER_LEFT),in,millis());
   if(allowed){manualRelease_=cmd;manualPending_=true;}
   else radio_.completeApplicationAction(cmd,generation_,false,2,0);
  }
 }
 in=boatHardware_inputs(sample_,millis(),executor_.stopped());
 in.primaryFresh=radio_.primaryFresh(millis());in.backupFresh=radio_.backupFresh(millis());
 if(radio_.missionPhase()==MC_MANUAL_REQUESTED){executor_.cancelManual();radio_.confirmManual();}
 const auto phase=radio_.missionPhase();
 const bool missionActive=phase>=MC_PREPARED&&phase<=MC_RETURNING;
 const auto intent=radio_.missionIntent();
 const uint16_t tx=radio_.missionTransaction();
 if(!missionActive)releaseTransaction_=0;
 if(missionActive&&tx!=releaseTransaction_){executor_.newMissionIdentity();releaseTransaction_=tx;}
 if(intent.intent==INTENT_RELEASE&&missionActive){
  if(!executor_.beginRelease(tx,intent.token,intent.channel,in,millis())){
   radio_.missionReleaseComplete(tx,intent.token,false);fault();return;
  }
 }
 int16_t left=0,right=0;const bool overrideActive=radio_.missionOverrideOutput(left,right);
 // The driver must check and enforce the output lease independently.
 const auto out=executor_.tick(in,millis(),intent,radio_.missionNavigation(),missionActive,overrideActive,left,right,sample_.safetyAt);
 if(!driver_.apply(out,100)){fault();return;}
 const auto completion=executor_.takeCompletion();
 if(completion.valid){if(manualPending_)finishManual(completion.success);else radio_.missionReleaseComplete(completion.transaction,completion.token,completion.success);}
 if(manualPending_&&executor_.stopped())finishManual(false);
 if(millis()-lastDataAt_>=500U){lastDataAt_=millis();publishData(lastDataAt_);}
 if(millis()-lastStatusAt_>=200U){lastStatusAt_=millis();radio_.sendHardwareTelemetry(boatHardware_status(sample_,lastStatusAt_,executor_.stopped(),radio_.missionPhase(),radio_.detachedReturnActive(),radio_.primaryFresh(lastStatusAt_)));}
}
