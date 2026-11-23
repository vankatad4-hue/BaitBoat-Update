#include "boat_application_port.h"
#include "boat_hardware_status.h"
#include "boat_hardware_telemetry.h"
#include "boat_shared_supply_fault_guard_r30b.h"
BoatApplicationPort *BoatApplicationPort::active_=nullptr;
bool BoatApplicationPort::attach(const BoatPhysicalDriver &driver,const BoatActuatorConfig &config){
 if(attached_||active_||!driver.read||!driver.apply||!driver.disable)return false;
 driver.disable();
 outputOffFeedback_.commandedDisable(millis());
 if(!executor_.configure(config))return false;
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
 return active_->executor_.stopped()&&!in.stopActive&&in.stationary&&in.releaseIdle&&in.releaseInterlocks&&
        in.fix.batteryHealthy&&active_->sample_.batteryBand<BOAT_BATTERY_RED;
}
void BoatApplicationPort::finishManual(bool success){
 if(!manualPending_)return;
 manualPending_=false;
 const uint32_t now=millis();
 const auto status=boatHardware_status(sample_,now,executor_.stopped(),radio_.missionPhase(),radio_.detachedReturnActive(),radio_.primaryFresh(now));
 radio_.completeApplicationAction(manualRelease_,generation_,success,status.stopState,(status.safetyFlags&1)?1:0);
}
void BoatApplicationPort::publishData(uint32_t now){
 const auto b=boatHardware_telemetry(sample_,now,++sampleSequence_,255,radio_.missionPhase());
 for(uint8_t i=0;i<b.count;++i)radio_.sendHardwareTelemetry(b.payload[i]);
}
void BoatApplicationPort::fault(){
 // In a fault path a same-tick pre-STOP sensor sample CANNOT confirm a
 // newly commanded physical STOP. Return an error/UNKNOWN to any pending
 // one-shot action, rather than synthesizing stopped feedback from an ACK.
 if(manualPending_){
  manualPending_=false;
  radio_.completeApplicationAction(manualRelease_,generation_,false,2,0);
 }
 executor_.stop();executor_.requireManualNeutral();
 radio_.localEmergencyStop();releaseTransaction_=0;
 if(driver_.disable){
  driver_.disable();
  outputOffFeedback_.commandedDisable(millis());
 }
}
void BoatApplicationPort::tick(){
 // Diagnostic default: poll and discard, never simulate a physical input.
 if(!attached_){
  radio_.update();BoatCommand cmd{};bool duplicate=false;ManagedLink link=LINK_NONE;
  while(radio_.takeCommand(cmd,duplicate,link)){}
  return;
 }
 // R30H-25: after apply() fails, the physical output state is uncertain.
 // Reassert physical disable, keep this instance irrevocably STOP-latched,
 // and drain/reject any subsequent command. A radio handshake, fresh sensor
 // or good battery cannot rearm a failed physical driver.
 if(driverFaultLatched_){
  driver_.disable();
  radio_.update();
  BoatCommand cmd{};bool duplicate=false;ManagedLink link=LINK_NONE;
  while(radio_.takeCommand(cmd,duplicate,link)){
   if(!duplicate&&cmd.type!=CMD_EMERGENCY_STOP)
    radio_.completeApplicationAction(cmd,radio_.sessionGeneration(),false,2,0,link);
  }
  radio_.sendHardwareTelemetry(boatHardware_status({},millis(),true,MC_ABORTED,false,false));
  return;
 }
 sample_={};sampleOK_=driver_.read(sample_);
 if(!sampleOK_){fault();radio_.update();radio_.sendHardwareTelemetry(boatHardware_status({},millis(),true,MC_ABORTED,false,false));return;}
 const uint32_t now=millis();
 const bool physicalBatteryHealthy=sample_.batteryHealthy;
 const bool batterySampleFresh=sample_.voltageValid&&(uint32_t)(now-sample_.batteryAt)<=BoatBatteryMonitor::MAX_SAMPLE_AGE_MS;
 const BoatBatterySnapshot battery=batteryMonitor_.update(batterySampleFresh,sample_.batteryMillivolts,now);
 // R30B: defense-in-depth against a shared-3S rail brownout, invalid I2C
 // conversion, stale battery timestamp or false battery-healthy flag.
 // The guard only DENIES unsafe operation; it cannot commission hardware.
 const auto energy=boat_r30b::inspectBoatBattery(
    battery,physicalBatteryHealthy,sample_.voltageValid,
    sample_.batteryMillivolts,sample_.batteryAt,now);
 sample_.batteryBand=battery.band;sample_.batteryPercent=battery.percent;sample_.batteryRthRequested=battery.rthRequested;
 sample_.batteryHealthy=energy!=boat_r30b::EnergyEvidence::STOP_INVALID_OR_CRITICAL;
 sample_.fix.batteryHealthy=sample_.batteryHealthy;
 // A genuine locally sampled motor/coil output still active after the
 // previous OFF command has exceeded its settle deadline is a persistent
 // hardware/feedback contradiction. Lock STOP, request disable again and
 // offer UNKNOWN STATUS. No new radio/clear-stop command may pass.
 if(outputOffFeedback_.mismatch(sample_,now)){
  driverFaultLatched_=true; // R30H-26 persistent reported output after OFF
  fault();
  radio_.update();
  radio_.sendHardwareTelemetry(boatHardware_status(sample_,millis(),true,MC_ABORTED,false,false));
  return;
 }
 auto in=boatHardware_inputs(sample_,now,executor_.stopped());
 if(!sample_.physicalStopReleased||!sample_.watchdogReady||!in.fix.batteryHealthy){fault();radio_.update();radio_.sendHardwareTelemetry(boatHardware_status(sample_,millis(),true,MC_ABORTED,false,radio_.primaryFresh(millis())));return;}
 radio_.update();
 const MissionControlPhase batteryPhase=radio_.missionPhase();
 const bool batteryMission=batteryPhase==MC_OUTBOUND||batteryPhase==MC_PAUSED||batteryPhase==MC_RETURNING||batteryPhase==MC_OVERRIDE_NEUTRAL||batteryPhase==MC_OVERRIDE_ACTIVE||batteryPhase==MC_RELEASING;
 if(sample_.batteryRthRequested&&batteryMission&&batteryPhase!=MC_RETURNING){
  if(batteryPhase==MC_RELEASING&&executor_.releaseBusy())executor_.cancelReleaseForFailsafe(now);
  else if(!executor_.releaseBusy()&&in.releaseIdle&&!radio_.requestLowBatteryReturnHome()){fault();return;}
 }
 if(generation_!=radio_.sessionGeneration()){
  // A normal radio re-handshake must not permanently fault the physical
  // driver: the radio watchdog has already commanded zero outputs. An
  // in-flight solenoid action is different; its result is uncertain, so latch
  // STOP and abort rather than replaying it after reconnection.
  if(manualPending_||executor_.releaseBusy()){
   manualPending_=false;executor_.stop();radio_.localEmergencyStop();
  }else{
   // Do not force a neutral handshake on initial application attachment;
   // subsequent authenticated session/epoch changes must always re-arm
   // manual control with a fresh neutral input before nonzero sticks.
   if(generation_!=0U)executor_.requireManualNeutral();
   else executor_.cancelManual();
  }
  generation_=radio_.sessionGeneration();
 }
 in=boatHardware_inputs(sample_,millis(),executor_.stopped());
 in.primaryFresh=radio_.primaryFresh(millis());in.backupFresh=radio_.backupFresh(millis());
 in.autonomousRth=radio_.failsafeReturnHomeActive();
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
  if(cmd.type==CMD_SET_MODE){
   // Previously this command could be ACKed as queued but then discarded.
   // Home may be requested only for a locally validated *running* mission.
   // The Boat cannot infer a safe new Home mission from a radio mode byte.
   const bool linked=(link==LINK_NRF_PRIMARY&&in.primaryFresh)||
                     (link==LINK_LORA_BACKUP&&in.backupFresh);
   const bool ok=cmd.mode==MODE_RETURN_HOME&&linked&&
                 !executor_.stopped()&&radio_.requestReturnHome();
   radio_.completeApplicationAction(cmd,generation_,ok,2,0,link);
   continue;
  }
  const auto phase=radio_.missionPhase();
  const bool manualMode=phase==MC_IDLE||phase==MC_COMPLETE||phase==MC_ABORTED;
  if(cmd.type==CMD_MANUAL_MOTORS&&manualMode&&!manualPending_)
   executor_.manual(cmd.leftMotor,cmd.rightMotor,cmd.createdAtMs,link==LINK_NRF_PRIMARY&&in.primaryFresh);
  if(cmd.type>=CMD_RELEASE_HOPPER_LEFT&&cmd.type<=CMD_RELEASE_HOOK_RIGHT){
   in=boatHardware_inputs(sample_,millis(),executor_.stopped());in.primaryFresh=radio_.primaryFresh(millis());
   in.autonomousRth=radio_.failsafeReturnHomeActive();
   const bool allowed=manualMode&&!manualPending_&&link==LINK_NRF_PRIMARY&&cmd.activate&&
     executor_.beginManualRelease((uint16_t)cmd.commandId,(int8_t)(cmd.type-CMD_RELEASE_HOPPER_LEFT),in,millis());
   if(allowed){manualRelease_=cmd;manualPending_=true;}
   else radio_.completeApplicationAction(cmd,generation_,false,2,0);
  }
 }
 in=boatHardware_inputs(sample_,millis(),executor_.stopped());
 in.primaryFresh=radio_.primaryFresh(millis());in.backupFresh=radio_.backupFresh(millis());
 in.autonomousRth=radio_.failsafeReturnHomeActive();
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
 if(!driver_.apply(out,100)){
  driverFaultLatched_=true; // never trust subsequent radio/sensor rearm
  fault();return;
 }
 outputOffFeedback_.acceptedOutput(out,millis());
 const auto completion=executor_.takeCompletion();
 if(completion.valid){if(manualPending_)finishManual(completion.success);else if(!sample_.batteryRthRequested)radio_.missionReleaseComplete(completion.transaction,completion.token,completion.success);}
 if(manualPending_&&executor_.stopped())finishManual(false);
 if(sample_.batteryRthRequested&&batteryMission&&radio_.missionPhase()!=MC_RETURNING&&!executor_.releaseBusy()&&
    boatHardware_inputs(sample_,millis(),executor_.stopped()).releaseIdle){
  if(!radio_.requestLowBatteryReturnHome()){fault();return;}
 }
 if(millis()-lastDataAt_>=500U){lastDataAt_=millis();publishData(lastDataAt_);}
 if(millis()-lastStatusAt_>=200U){lastStatusAt_=millis();radio_.sendHardwareTelemetry(boatHardware_status(sample_,lastStatusAt_,executor_.stopped(),radio_.missionPhase(),radio_.detachedReturnActive(),radio_.primaryFresh(lastStatusAt_)));}
}
