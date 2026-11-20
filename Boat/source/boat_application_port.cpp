#include "boat_application_port.h"
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
void BoatApplicationPort::fault(){
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
 if(!sampleOK_){fault();radio_.update();return;}
 const uint32_t now=millis();
 auto in=boatHardware_inputs(sample_,now,executor_.stopped());
 if(!sample_.physicalStopReleased||!sample_.watchdogReady||!in.fix.batteryHealthy){fault();radio_.update();return;}
 radio_.update();
 in=boatHardware_inputs(sample_,millis(),executor_.stopped());
 in.primaryFresh=radio_.primaryFresh(millis());in.backupFresh=radio_.backupFresh(millis());
 BoatCommand cmd{};bool duplicate=false;ManagedLink link=LINK_NONE;
 while(radio_.takeCommand(cmd,duplicate,link)){
  if(cmd.type==CMD_EMERGENCY_STOP){fault();return;}
  if(duplicate)continue;
  if(cmd.type==CMD_CLEAR_STOP){
   // Remote clear-STOP API checks calibrated neutral sticks; local driver
   // independently confirms stationary propulsion and all interlocks.
   executor_.clearStop(sample_,millis(),link==LINK_NRF_PRIMARY&&in.primaryFresh,in.stationary);
   continue;
  }
  if(cmd.type==CMD_MANUAL_MOTORS&&radio_.missionPhase()!=MC_MANUAL_REQUESTED)
   executor_.manual(cmd.leftMotor,cmd.rightMotor,cmd.createdAtMs,link==LINK_NRF_PRIMARY&&in.primaryFresh);
  // Manual mode transitions/releases need their own physical completion
  // feedback and are intentionally not executed by this first bridge.
 }
 in=boatHardware_inputs(sample_,millis(),executor_.stopped());
 in.primaryFresh=radio_.primaryFresh(millis());in.backupFresh=radio_.backupFresh(millis());
 const auto phase=radio_.missionPhase();
 const bool missionActive=phase>=MC_PREPARED&&phase<=MC_RETURNING;
 const auto intent=radio_.missionIntent();
 const uint16_t tx=radio_.missionTransaction();
 if(missionActive&&tx!=releaseTransaction_){executor_.newMissionIdentity();releaseTransaction_=tx;}
 if(intent.intent==INTENT_RELEASE&&missionActive){
  if(!executor_.beginRelease(tx,intent.token,intent.channel,in,millis())){
   radio_.missionReleaseComplete(tx,intent.token,false);fault();return;
  }
 }
 int16_t left=0,right=0;const bool overrideActive=radio_.missionOverrideOutput(left,right);
 // The driver must check and enforce the output lease independently.
 const auto out=executor_.tick(in,millis(),intent,radio_.missionNavigation(),missionActive,overrideActive,left,right);
 if(!driver_.apply(out,100)){fault();return;}
 const auto completion=executor_.takeCompletion();
 if(completion.valid)radio_.missionReleaseComplete(completion.transaction,completion.token,completion.success);
}
