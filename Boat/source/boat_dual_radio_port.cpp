#include "boat_dual_radio_port.h"
#include "radio_payload_codec.h"
#include <string.h>


// Provisional per-link FIFO dispatch budget; not a measured STOP deadline.
static constexpr unsigned RX_FRAME_BUDGET=4;
static constexpr uint8_t REMOTE_ID=0x01,BOAT_ID=0x02;

BoatDualRadioPort::BoatDualRadioPort()
 : nrf_(BOAT_NRF_CE_PIN,BOAT_NRF_CSN_PIN,&SPI,BOAT_NRF_CHANNEL,BOAT_NRF_SCK_PIN,BOAT_NRF_MISO_PIN,BOAT_NRF_MOSI_PIN),
   e22_(e22Serial_),nrfReady_(false),sx1262Ready_(false),sessionEpoch_(1),
   lastNrfRxMs_(0),lastBackupRxMs_(0),lastNrfHeartbeatTxMs_(0),
   lastBackupHeartbeatTxMs_(0),lastBackupHeartbeatAttemptMs_(0),txSequence_(0),authenticated_(false),
   sessionStarted_(false),lastNrfRxValid_(false),lastBackupRxValid_(false) {

 memset(key_,0,sizeof(key_));memset(rootKey_,0,sizeof(rootKey_));
}
void BoatDualRadioPort::abortMission(){
 controller_.emergencyAbort();stickLease_.reset();missionIntent_={INTENT_STOP,-1,0};
}
bool BoatDualRadioPort::missionInputs(ExecInputs &in,uint32_t now)const{
 in={};if(!missionInput_||!missionInput_(in))return false;
 in.primaryFresh=primaryFresh(now);in.backupFresh=backupFresh(now);in.autonomousRth=failsafeRthActive_;return true;
}
bool BoatDualRadioPort::missionOverrideOutput(int16_t &left,int16_t &right)const{
 ExecInputs in{};const uint32_t now=millis();const bool ready=missionInputs(in,now);
 return stickLease_.output(controller_.status().transaction,now,ready&&controller_.temporaryOverrideConfirmed()&&in.primaryFresh&&!in.stopActive&&in.releaseIdle&&in.releaseInterlocks,left,right);
}
bool BoatDualRadioPort::missionReleaseComplete(uint16_t transaction,uint16_t token,bool success){
 ExecInputs in{};const bool ready=missionInputs(in,millis());
 const bool safe=ready&&in.primaryFresh&&!in.stopActive&&in.releaseInterlocks&&in.releaseIdle&&in.stationary;
 return controller_.releaseComplete(transaction,token,success&&safe,safe);
}
bool BoatDualRadioPort::sendHardwareTelemetry(const RadioDecodedPayload &status){
 return sendHardwareTelemetry(status,millis());
}
bool BoatDualRadioPort::sendHardwareTelemetry(const RadioDecodedPayload &status,uint32_t sampledAt){
 const uint32_t now=millis();
 if(sessionReady()&&backupFresh(now)&&status.kind==RADIO_PAYLOAD_STATUS){
  if(status.stopState==1U)backupStatusOutbox_.offer(status,now);
  else backupStatusOutbox_.reset();
 }
 if(!sessionReady()||!primaryFresh(now)){
  optionalDiagnostics_.discard();return false;
 }
 if(status.kind==RADIO_PAYLOAD_STATUS){
  // R30H-19: schedule only the most recent real output-feedback snapshot.
  // Publish it AFTER a pending STOP ACK, ahead of heartbeat/optional data.
  return statusOutbox_.offer(status,now); // local queue, not radio delivery
 }
 if(status.kind==RADIO_PAYLOAD_SAFETY_DIAGNOSTIC){
  // R30G: strictly optional telemetry is a REPLACEABLE one-slot offer.
  // Do not allocate a RadioFrame, advance sequence or occupy nRF TX here.
  // The radio update() sends it only AFTER heartbeat/ACK/mission duties.
  uint8_t payload[8]{};
  if(!radio_encodeTelemetryPayload(status,payload))return false;
  return optionalDiagnostics_.offer(payload,now);
 }
#if BOAT_R30H121_ENDPOINT_COMPILE
 if(BOAT_R30H121_PHYSICAL_OWNER){
  uint8_t payload[8]{};
  return uint32_t(now-sampledAt)<flytec_r30h123::TelemetrySlots::MAX_AGE_MS&&radio_encodeTelemetryPayload(status,payload)&&asyncTelemetry_.offer(payload,sampledAt);
 }
#endif
 if(!primaryOrdinaryTxReady())return false; // Never burn a sequence on busy TX.
 RadioFrame f;radio_frameInit(f,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
 return radio_encodeTelemetryPayload(status,f.payload)&&radio_frameSeal(f,key_)&&sendFrame(LINK_NRF_PRIMARY,f);
}
bool BoatDualRadioPort::sendMissionFeedback(const MissionControlFeedback &status,uint32_t now){
 // Never spend LoRa airtime on 250-ms status updates while nRF is healthy.
 // On authenticated backup-only operation, return mission feedback via E22.
 const ManagedLink link=primaryFresh(now)?LINK_NRF_PRIMARY:
                        (backupFresh(now)?LINK_LORA_BACKUP:LINK_NONE);
 if(link==LINK_NONE||!sessionReady())return false;
 if(link==LINK_LORA_BACKUP&&status.result!=MC_STATUS){
  // Never overwrite an older unacknowledged correlated response with a
  // different request. Remote will retry the newer request after this slot.
  if(pendingBackupMissionFeedbackValid_){
   // Duplicate requests must not refresh expiry or overwrite the original
   // correlated result, even if the mission phase has since changed.
   return false;
  }
  pendingBackupMissionFeedback_=status;
  pendingBackupMissionFeedbackValid_=true;
  pendingBackupMissionFeedbackAtMs_=now;
 }
 if(link==LINK_LORA_BACKUP&&pendingBackupMissionFeedbackValid_&&status.result==MC_STATUS)return false;
#if BOAT_R30H121_ENDPOINT_COMPILE
 // The owner cycle retries this retained reply using its original deadline.
 // Queue acceptance must not erase a correlated backup response.
 if(runtimeOwner_.worker()&&link==LINK_LORA_BACKUP&&status.result!=MC_STATUS)return true;
#endif
 if(link==LINK_NRF_PRIMARY&&!primaryOrdinaryTxReady())return false;
 RadioFrame f;radio_frameInit(f,RADIO_MSG_MISSION_FEEDBACK,BOAT_ID,REMOTE_ID,++txSequence_,status.transaction,now,sessionEpoch_);
 const bool sent=missionFeedbackEncode(status,f.payload)&&radio_frameSeal(f,key_)&&sendFrame(link,f);
 if(sent&&link==LINK_LORA_BACKUP&&status.result!=MC_STATUS)pendingBackupMissionFeedbackValid_=false;
 return sent;
}
bool BoatDualRadioPort::requestLowBatteryReturnHome(){
 const auto phase=controller_.status().phase;
 if(failsafeRthActive_||detachedReturn_||phase==MC_RETURNING||!controller_.canDivertHome())return false;
 BoatHomePoint home{};
 if(!homePointStore_.load||!homePointStore_.load(home)||!boatHomePointValid(home)||!controller_.homeMatches(home.latE7,home.lonE7))return false;
 ExecInputs in{};
 if(!missionInputs(in,millis())||in.stopActive||!in.batteryLow||!in.releaseIdle||!in.releaseInterlocks||!controller_.homeNavigationValid(in))return false;
 if(!controller_.divertHome(in,true))return false;
 failsafeRthActive_=true;detachedReturn_=true;linkLossHold_=false;stickLease_.reset();
 missionIntent_={INTENT_STOP,-1,0};
 return true;
}
bool BoatDualRadioPort::requestReturnHome(){
 // Explicit Home still requires a locally stored Home record matching the
 // prepared plan. No new mission may be created by a backup radio command.
 BoatHomePoint storedHome{};
 if(!homePointStore_.load||!homePointStore_.load(storedHome)||
    !boatHomePointValid(storedHome)||!controller_.homeMatches(storedHome.latE7,storedHome.lonE7))return false;
 const auto phase=controller_.status().phase;
 if(!sessionReady()||detachedReturn_||
    (phase!=MC_OUTBOUND&&phase!=MC_PAUSED&&phase!=MC_RETURNING&&
     phase!=MC_OVERRIDE_NEUTRAL&&phase!=MC_OVERRIDE_ACTIVE))return false;
 ExecInputs in{};
 if(!missionInputs(in,millis())||in.stopActive||!in.releaseIdle||
    (!in.primaryFresh&&!in.backupFresh))return false;
 if(!controller_.divertHome(in,true))return false;
 detachedReturn_=true;stickLease_.reset();
 missionIntent_={INTENT_STOP,-1,0}; // one safe tick before Home navigation
 return true;
}
void BoatDualRadioPort::resetSession(bool preserveMission){
 ++generation_;releaseIdSeen_=false;
 const auto phase=controller_.status().phase;
 const bool resumable=preserveMission&&
   (phase==MC_OUTBOUND||phase==MC_RETURNING||phase==MC_PAUSED);
 if(resumable){
  // Keep only an already-running autonomous plan. Transport queues, stick
  // leases and actuator requests are discarded; output remains STOP until a
  // new authenticated heartbeat proves that a radio has returned.
  linkLossHold_=true;stickLease_.reset();missionIntent_={INTENT_STOP,-1,0};
 }else{
  linkLossHold_=false;detachedReturn_=false;failsafeRthActive_=false;
  abortMission();controller_.resetSession();stickLease_.reset();
  missionIntent_={INTENT_STOP,-1,0};
  haveAuthenticatedRx_=false;lastAnyAuthenticatedRxMs_=0;
 }
 handshake_.reset();radioSession_wipe(key_,32);sessionStarted_=false;
#if BOAT_R30H121_ENDPOINT_COMPILE
 if(BOAT_R30H121_PHYSICAL_OWNER){asyncPrimaryTx_.reset();asyncBackupTx_.reset();asyncTelemetry_.reset();asyncPrimaryCycle_.close();asyncBackupCycle_.close();}
#endif
 primarySeqGuard_.resetAfterHandshake();backupSeqGuard_.resetAfterHandshake();crossRadioSeqGuard_.resetAfterHandshake();
 lastNrfRxValid_=lastBackupRxValid_=false;commandQueue_.clear();mission_.reset();dedupe_.clear();txSequence_=0;
 // R30G: never carry a diagnostic offer or stale activity timestamp across
 // authenticated session/key/epoch rollover.
 optionalDiagnostics_.reset();lastControlRxValid_=false;
 backupStopAck_.reset();backupStatusOutbox_.reset();
 stopAck_.reset(); // Never replay a STOP ACK across session/epoch boundaries.
 statusOutbox_.reset(); // Pre-session status is not an authority.
 pendingBackupMissionFeedbackValid_=false;lastBackupMissionFeedbackAttemptMs_=0;
 havePrimaryHeartbeatSent_=false;
 utcSource_={};utcEverSent_=false;lastUtcTxMs_=0;
 radioWatchdog_.reset();
}
bool BoatDualRadioPort::begin() {
 // One initialization attempt per object; no implicit restart after partial failure.
 if(legacyBeginAttempted_||!runtimeOwner_.claimLegacy())return false;
 legacyBeginAttempted_=true;
 resetSession();nrfReady_=false;sx1262Ready_=false;authenticated_=radioKey_get(rootKey_);
 lastNrfHeartbeatTxMs_=lastBackupHeartbeatTxMs_=lastBackupHeartbeatAttemptMs_=0;
// The external E22 UART and nRF SPI use separate peripherals.
// Every pin remains -1 until an audited hardware profile is supplied.
#if BOAT_NRF_PIN_AUDIT_SIGNED && BOAT_NRF_CE_PIN >= 0 && BOAT_NRF_CSN_PIN >= 0 && BOAT_NRF_SCK_PIN >= 0 && BOAT_NRF_MISO_PIN >= 0 && BOAT_NRF_MOSI_PIN >= 0
 nrfReady_=nrf_.begin(false);
#endif
#if BOAT_E22_UART_PIN_TX >= 0 && BOAT_E22_UART_PIN_RX >= 0 && BOAT_E22_UART_PIN_M0 >= 0 && BOAT_E22_UART_PIN_M1 >= 0 && BOAT_E22_UART_PIN_AUX >= 0 && BOAT_E22_UART_PIN_NRST >= 0 && BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED && BOAT_E22_UART_LOGIC_LEVELS_VERIFIED && BOAT_E22_UART_PIN_AUDIT_SIGNED && BOAT_E22_RF_AIRTIME_AUDIT_SIGNED
 const E22BoardPins pins={(int8_t)BOAT_E22_UART_PIN_TX,(int8_t)BOAT_E22_UART_PIN_RX,
  (int8_t)BOAT_E22_UART_PIN_M0,(int8_t)BOAT_E22_UART_PIN_M1,(int8_t)BOAT_E22_UART_PIN_AUX,
  (int8_t)BOAT_E22_UART_PIN_NRST,true,true,true};
 sx1262Ready_=e22_.begin(pins);
#endif

 // Wait for the remote's authenticated session-start marker. Its epoch is the
 // session identifier used by both endpoints; do not invent a boat-local epoch.
 return nrfReady_||sx1262Ready_;
}
void BoatDualRadioPort::update() {
 const uint32_t now=millis();
 prepareApplicationCycle(now);
#if BOAT_R30H121_ENDPOINT_COMPILE
 if(runtimeOwner_.worker()){
  serviceAsyncRuntime(now);
  return;
 }
#endif
 serviceLegacyRxAndFeedback();
 advanceApplicationCycle(millis());
 serviceLegacyPeriodicTx(now);
}
void BoatDualRadioPort::prepareApplicationCycle(uint32_t now){
 // Core 1 application preparation; no physical driver calls.

 // Radio inactivity is handled by BoatRadioWatchdog, not by tearing down
 // the authenticated session. Keeping the session lets the backup E22 link
 // recover after a long RF outage without requiring the primary radio.
 // Sequence-limit rekey still uses the existing primary handshake.
 if(sessionStarted_&&!sessionReady())resetSession(true);
 if(handshake_.timeout(now))resetSession(linkLossHold_||detachedReturn_);
 // Primary-only actions and stick leases must never survive primary-link loss.
 if(!primaryFresh(now)){
  stickLease_.reset();
  commandQueue_.discardLink((uint8_t)LINK_NRF_PRIMARY,true);
 }
 if(!missionGate_||!missionGate_()||(!primaryFresh(now)&&!backupFresh(now)))mission_.reset();
}
void BoatDualRadioPort::serviceLegacyRxAndFeedback(){
 if(!legacyBeginAttempted_||!runtimeOwner_.legacy())return;
 if(nrfReady_){
  const bool stopAckWasInFlight=stopAck_.inFlight();
  nrf_.update();
  if(stopAckWasInFlight&&!primaryDriverPending())
   stopAck_.completed(primaryDriverReady()&&primaryDriverSuccess());
  if(!primaryDriverReady()){
   stopAck_.reset(); // A failed PHY cannot retain a response from a dead session.
   statusOutbox_.reset();
   // R30H-13: revoke primary immediately on its transport watchdog fault.
   // Queued/stick primary commands cannot survive a transport failure.
   nrfReady_=false;lastNrfRxValid_=false;
   stickLease_.reset();mission_.reset();
   controller_.cancelOverrideForLinkLoss();
   commandQueue_.discardLink((uint8_t)LINK_NRF_PRIMARY,true);
  } else {
   RadioFrame f;
   for(unsigned i=0;i<RX_FRAME_BUDGET&&receivePrimaryLegacy(f);++i)receive(LINK_NRF_PRIMARY,f);
   // R30H-18: after RX accepts STOP, its correlated ACK claims the first
   // available primary TX slot. Never let heartbeat/status/diagnostics
   // get ahead of an outstanding STOP response.
   const uint32_t ackNow=millis();
   if(stopAck_.due(ackNow,sessionReady(),nrfReady_,primaryDriverPending())){
    const auto pending=stopAck_.ticket();
    if(pending.epoch==sessionEpoch_){
     RadioFrame reply{};
     radio_frameInit(reply,RADIO_MSG_ACK,BOAT_ID,REMOTE_ID,
                     ++txSequence_,pending.commandId,ackNow,sessionEpoch_);
     if(radio_encodeAckPayload(pending.result,CMD_EMERGENCY_STOP,
                               pending.commandId,pending.stopState,
                               pending.interlocks,reply.payload)&&
        radio_frameSeal(reply,key_)&&admitPrimaryLegacy(reply))
      stopAck_.admitted(millis()); // admission only, NOT Remote receipt
    }else stopAck_.reset();
   }
   // The single latest STATUS can now use the first free nRF TX slot.
   // Unlike STOP ACK, this snapshot must come from BoatHardware_status()
   // (real local feedback) and expires quickly if the slot stays busy.
   const uint32_t statusNow=millis();
   if(statusOutbox_.due(statusNow,sessionReady(),nrfReady_,
                        primaryDriverPending(),stopAck_.pending())){
    RadioFrame report{};
    radio_frameInit(report,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,
                    ++txSequence_,0,statusNow,sessionEpoch_);
    if(radio_encodeTelemetryPayload(statusOutbox_.status(),report.payload)&&
       radio_frameSeal(report,key_)&&admitPrimaryLegacy(report))
       statusOutbox_.sent(); // TX started; not an actuator/safety ACK
   }
  }
 }
 if(sx1262Ready_){
  e22_.update();
  if(backupDriverFault()){
   // R30H-116 closeout: local UART fault must revoke stale backup health.
   sx1262Ready_=false;lastBackupRxValid_=false;backupStopAck_.reset();backupStatusOutbox_.reset();
   pendingBackupMissionFeedbackValid_=false;
   commandQueue_.discardLink((uint8_t)LINK_LORA_BACKUP,true);
  }else{
   RadioFrame f;for(unsigned i=0;i<RX_FRAME_BUDGET&&receiveBackupLegacy(f);++i)receive(LINK_LORA_BACKUP,f);
  }
 }
 // Backup STOP ACK claims the first available UART admission slot.
 const uint32_t backupAckNow=millis();
 if(backupStopAck_.due(backupAckNow,sessionReady(),sx1262Ready_,!backupDriverReady())){
  const auto pending=backupStopAck_.ticket();
  if(pending.epoch!=sessionEpoch_)backupStopAck_.reset();
  else{
   RadioFrame reply{};
   radio_frameInit(reply,RADIO_MSG_ACK,BOAT_ID,REMOTE_ID,++txSequence_,pending.commandId,backupAckNow,sessionEpoch_);
   reply.flags=RADIO_FLAG_BACKUP_LINK;
   if(radio_encodeAckPayload(pending.result,CMD_EMERGENCY_STOP,pending.commandId,pending.stopState,pending.interlocks,reply.payload)&&
      radio_frameSeal(reply,key_)&&admitBackupLegacy(reply,backupAckNow))backupStopAck_.reset(); // UART admission only; Remote retries cover loss
  }
 }
 const uint32_t backupStatusNow=millis();
 if(backupStatusOutbox_.due(backupStatusNow,sessionReady(),sx1262Ready_,!backupDriverReady(),backupStopAck_.pending())&&
    (uint32_t)(backupStatusNow-lastBackupHeartbeatTxMs_)>=3000U){
  RadioFrame report{};
  radio_frameInit(report,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,++txSequence_,0,backupStatusNow,sessionEpoch_);
  report.flags=RADIO_FLAG_BACKUP_LINK;
  if(radio_encodeTelemetryPayload(backupStatusOutbox_.status(),report.payload)&&radio_frameSeal(report,key_)&&admitBackupLegacy(report,backupStatusNow)){
   backupStatusOutbox_.sent();lastBackupHeartbeatTxMs_=backupStatusNow;
  }
 }
 // Correlated feedback outranks periodic status/heartbeat when E22 was busy.
 if(pendingBackupMissionFeedbackValid_){
  const uint32_t retryNow=millis();
  if(!sessionReady()||(uint32_t)(retryNow-pendingBackupMissionFeedbackAtMs_)>12000U)
   pendingBackupMissionFeedbackValid_=false;
  else if(backupFresh(retryNow)&&
          (!lastBackupMissionFeedbackAttemptMs_||(uint32_t)(retryNow-lastBackupMissionFeedbackAttemptMs_)>=250U)){
   lastBackupMissionFeedbackAttemptMs_=retryNow;
   const MissionControlFeedback pending=pendingBackupMissionFeedback_;
   RadioFrame reply{};
   radio_frameInit(reply,RADIO_MSG_MISSION_FEEDBACK,BOAT_ID,REMOTE_ID,++txSequence_,pending.transaction,retryNow,sessionEpoch_);
   if(missionFeedbackEncode(pending,reply.payload)&&radio_frameSeal(reply,key_)&&sendFrame(LINK_LORA_BACKUP,reply))
    pendingBackupMissionFeedbackValid_=false;
  }
 }

}
void BoatDualRadioPort::advanceApplicationCycle(uint32_t safetyNow){
 const auto watchdogEvent=radioWatchdog_.update(primaryFresh(safetyNow),backupFresh(safetyNow));
 if(watchdogEvent==BOAT_RADIO_WATCHDOG_LOST){
  linkLossHold_=true;
  commandQueue_.clearNonEmergency();
  stickLease_.reset();
  missionIntent_={INTENT_STOP,-1,0};
#if BOAT_R30H_LOCAL_TRACE_ENABLED
  localTrace_.push(boat_r30h2::BOTH_LINKS_STALE,safetyNow,
                   lastAnyAuthenticatedRxMs_,0,0);
#endif
 }else if(watchdogEvent==BOAT_RADIO_WATCHDOG_RESTORED){
  // An authenticated fresh heartbeat/control frame has restored at least one
  // link. The retained autonomous mission may now continue; stale manual
  // commands and queued releases have already been discarded.
  linkLossHold_=false;
#if BOAT_R30H_LOCAL_TRACE_ENABLED
  const uint8_t fresh=(primaryFresh(safetyNow)?1U:0U)|
                      (backupFresh(safetyNow)?2U:0U);
  localTrace_.push(boat_r30h2::AUTH_LINK_RESTORED,safetyNow,
                   lastAnyAuthenticatedRxMs_,0,fresh);
#endif
 }
 if(!primaryFresh(safetyNow)){
  stickLease_.reset();
  controller_.cancelOverrideForLinkLoss();
  commandQueue_.discardLink((uint8_t)LINK_NRF_PRIMARY,true);
 }

 const auto progress=controller_.status();
 if(progress.transaction){
   ExecInputs in{};const bool ready=missionInputs(in,safetyNow);
   missionIntent_=controller_.tick(in,safetyNow,ready,stickLease_.neutral(progress.transaction,safetyNow));
   int16_t left=0,right=0;
   if(controller_.temporaryOverrideConfirmed()&&!missionOverrideOutput(left,right))abortMission();
   if(detachedReturn_&&!controller_.diverted()){detachedReturn_=false;failsafeRthActive_=false;controller_.resetSession();missionIntent_={INTENT_STOP,-1,0};}
   if(controller_.status().transaction&&safetyNow-lastMissionStatusMs_>=
       (primaryFresh(safetyNow)?250U:3000U)){
      lastMissionStatusMs_=safetyNow;sendMissionFeedback(controller_.status(),safetyNow);
    }
 }
 maybeStartFailsafeReturnHome(safetyNow);
}
void BoatDualRadioPort::serviceLegacyPeriodicTx(uint32_t now){
 if(!legacyBeginAttempted_||!runtimeOwner_.legacy())return;
 if(sessionReady()) {
   if(nrfReady_ && (uint32_t)(now-lastNrfHeartbeatTxMs_)>=100U) {
     // A busy TX must not advance the heartbeat schedule. The next update
     // retries the heartbeat before any R30G diagnostic opportunity.
     if(sendHeartbeat(LINK_NRF_PRIMARY,now)){
       lastNrfHeartbeatTxMs_=now;havePrimaryHeartbeatSent_=true;
     }
   }
   if(sx1262Ready_ && (uint32_t)(now-lastBackupHeartbeatTxMs_)>=3000U && (uint32_t)(now-lastBackupHeartbeatAttemptMs_)>=250U) {lastBackupHeartbeatAttemptMs_=now;if(sendHeartbeat(LINK_LORA_BACKUP,now))lastBackupHeartbeatTxMs_=now;}
 }
 // R30G optional diagnostic LAST: no queue of old payloads, no E22 path,
 // never preempts TX, pending STOP/command, active mission or failsafe.
 // Manual control activity blocks it until 250 ms after the last valid
 // command/mission-sticks frame. This is intentionally lossy telemetry.
 boat_r30g::Opportunity opportunity{};
 opportunity.authenticatedSession=sessionReady();
 opportunity.primaryFresh=primaryFresh(millis());
 opportunity.txPending=primaryDriverPending();
 opportunity.commandQueued=commandQueue_.size()!=0||stopAck_.pending()||statusOutbox_.pending();
 opportunity.activeMission=controller_.status().transaction!=0;
 opportunity.failsafeOrLinkLoss=linkLossHold_||detachedReturn_||failsafeRthActive_;
 opportunity.recentCommand=lastControlRxValid_ &&
      (uint32_t)(millis()-lastControlRxMs_)<boat_r30g::MIN_IDLE_AFTER_COMMAND_MS;
 opportunity.lastHeartbeatAtMs=lastNrfHeartbeatTxMs_;
 opportunity.haveHeartbeat=havePrimaryHeartbeatSent_;
 uint8_t optionalPayload[8]{};
 const uint32_t optionalNow=millis();
 const bool diagnosticOccupiesSlot=optionalDiagnostics_.take(opportunity,optionalNow,optionalPayload);
 if(diagnosticOccupiesSlot){
   // The scheduler has ALREADY enforced heartbeat/command priority.
   RadioFrame optional{};
   radio_frameInit(optional,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,
                   ++txSequence_,0,optionalNow,sessionEpoch_);
   for(unsigned i=0;i<8;++i)optional.payload[i]=optionalPayload[i];
   if(radio_frameSeal(optional,key_)&&sendFrame(LINK_NRF_PRIMARY,optional))
     optionalDiagnostics_.sent(optionalNow);
   else
     optionalDiagnostics_.rejectedTx(); // no retries or priority inversion
 }
 // R30H-5 UTC LAST: authenticated nRF only, no E22 airtime.
 // No TX during missions, command queue, manual control activity,
 // failsafe or a missed heartbeat. This time stream is informational and
 // CANNOT extend a link lease or change the STOP/RTH monotonic timers.
 const uint32_t utcNow=millis();
 const uint32_t gap=(uint32_t)(utcNow-lastNrfHeartbeatTxMs_);
 if(!diagnosticOccupiesSlot&&opportunity.authenticatedSession&&
    opportunity.primaryFresh&&opportunity.haveHeartbeat&&
    !opportunity.commandQueued&&!opportunity.activeMission&&
    !opportunity.failsafeOrLinkLoss&&!opportunity.recentCommand&&
    !primaryDriverPending()&&gap>=boat_r30g::HEARTBEAT_GAP_BEGIN_MS&&
    gap<=boat_r30g::HEARTBEAT_GAP_END_MS&&
    (!utcEverSent_||(uint32_t)(utcNow-lastUtcTxMs_)>=5000U)){
   uint64_t ms=0;
   if(flytec_utc::extrapolate(utcSource_,utcNow,ms)){
     RadioFrame beacon{};
     radio_frameInit(beacon,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,
                     ++txSequence_,0,utcNow,sessionEpoch_);
     if(flytec_time::encode(ms,beacon.payload)&&
        radio_frameSeal(beacon,key_)&&sendFrame(LINK_NRF_PRIMARY,beacon)){
       lastUtcTxMs_=utcNow;utcEverSent_=true;
     }
   }
 }
}
bool BoatDualRadioPort::maybeStartFailsafeReturnHome(uint32_t now){
 if(failsafeRthActive_||primaryFresh(now)||backupFresh(now)||!controller_.canDivertHome())return false;
 BoatHomePoint home{};
 const bool homeStored=homePointStore_.load&&homePointStore_.load(home)&&
   boatHomePointValid(home)&&controller_.homeMatches(home.latE7,home.lonE7);
 ExecInputs in{};const bool inputsReady=missionInputs(in,now);
 const bool navigationSafe=inputsReady&&!in.stopActive&&in.releaseIdle&&in.releaseInterlocks&&
   controller_.homeNavigationValid(in);
 if(!boatRadioLossRthDue(linkLossHold_&&!primaryFresh(now)&&!backupFresh(now),
      controller_.canDivertHome(),homeStored,navigationSafe,haveAuthenticatedRx_,
      lastAnyAuthenticatedRxMs_,now,10000U))return false;
 if(!controller_.divertHome(in,true))return false;
 // Keep outputs stopped for this update. The next application tick may issue
 // only the local INTENT_NAV_HOME after the actuator supervisor rechecks inputs.
 failsafeRthActive_=true;detachedReturn_=true;stickLease_.reset();
 missionIntent_={INTENT_STOP,-1,0};
#if BOAT_R30H_LOCAL_TRACE_ENABLED
 localTrace_.push(boat_r30h2::RTH_LOCAL_ENTRY,now,lastAnyAuthenticatedRxMs_,0,0);
#endif
 return true;
}

void BoatDualRadioPort::receive(ManagedLink link,const RadioFrame &f) {
 receiveCaptured(link,f,millis());
}
#if BOAT_R30H121_ENDPOINT_COMPILE
bool BoatDualRadioPort::beginAsyncRuntime(bool commissioned){
 // The compile-only profile cannot start hardware, even with caller consent.
 if(!BOAT_R30H121_PHYSICAL_OWNER||!commissioned||legacyBeginAttempted_||
    !runtimeOwner_.claimWorker(commissioned))return false;
 resetSession();nrfReady_=sx1262Ready_=false;
 authenticated_=radioKey_get(rootKey_);
 lastNrfHeartbeatTxMs_=lastBackupHeartbeatTxMs_=lastBackupHeartbeatAttemptMs_=0;
 if(!authenticated_||!asyncTask_.bindShared(asyncPrimaryHandshake_,asyncBackupHandshake_,
    asyncPrimaryPriority_,asyncBackupPriority_,BOAT_ID,REMOTE_ID)||
    !asyncTask_.start(commissioned,true)){
  stopAsyncRuntime();return false;
 }
 return true;
}
void BoatDualRadioPort::stopAsyncRuntime(){
 if(!runtimeOwner_.worker())return;
 asyncTask_.stop();
 runtimeOwner_.retire();
 nrfReady_=sx1262Ready_=false;lastNrfRxValid_=lastBackupRxValid_=false;
 resetSession(linkLossHold_||detachedReturn_);
 // Object and queues remain allocated. Task exit does not prove RF silence.
}
void BoatDualRadioPort::serviceAsyncRuntime(uint32_t now){
 using flytec_r30h120::Phase;
 const auto phase=asyncTask_.phase();
 if(phase==Phase::Failed||phase==Phase::Stopped||phase==Phase::Stopping){
  stopAsyncRuntime();advanceApplicationCycle(millis());return;
 }
 if(phase!=Phase::Running){advanceApplicationCycle(millis());return;}
 // Poll worker health before authenticated dispatch. Busy TX is not a
 // transport failure. Only authenticated RX can refresh the link lease.
 asyncRadio_.primary().pollHealth();asyncRadio_.backup().pollHealth();
 const bool p=asyncRadio_.primary().available(now,250U);
 const bool b=asyncRadio_.backup().available(now,250U);
 if(nrfReady_&&!p){
  lastNrfRxValid_=false;stickLease_.reset();mission_.reset();
  controller_.cancelOverrideForLinkLoss();
  commandQueue_.discardLink((uint8_t)LINK_NRF_PRIMARY,true);
 }
 if(sx1262Ready_&&!b){
  lastBackupRxValid_=false;pendingBackupMissionFeedbackValid_=false;
  commandQueue_.discardLink((uint8_t)LINK_LORA_BACKUP,true);
 }
 nrfReady_=p;sx1262Ready_=b;
 if(p)drainAsyncRx(LINK_NRF_PRIMARY,now);
 if(b)drainAsyncRx(LINK_LORA_BACKUP,now);
 advanceApplicationCycle(millis());
 serviceAsyncOwner(LINK_NRF_PRIMARY,millis(),p);
 serviceAsyncOwner(LINK_LORA_BACKUP,millis(),b);
}
bool BoatDualRadioPort::queueAsyncHandshake(ManagedLink link,const RadioFrame &frame,uint32_t now){
 if(!BOAT_R30H121_PHYSICAL_OWNER||!runtimeOwner_.worker()||legacyBeginAttempted_||!authenticated_)return false;
 if(link!=LINK_NRF_PRIMARY&&link!=LINK_LORA_BACKUP)return false;
 const bool backup=link==LINK_LORA_BACKUP;
 auto &owner=backup?asyncBackupOwner_:asyncPrimaryOwner_;
 auto shape=[&](const RadioFrame &f){return flytec_r30h120::boatReplyShape(f,BOAT_ID,REMOTE_ID,backup);};
 if(!shape(frame)||!radio_frameVerify(frame,rootKey_))return false;
 if(!owner.beginHandshake())return false;
 return owner.submitHandshake(frame,now,shape,[&](const RadioFrame &f){return radio_frameVerify(f,rootKey_);});
}
bool BoatDualRadioPort::queueAsyncApplication(ManagedLink link,const RadioFrame &frame,uint32_t now,uint32_t age){
 if(!BOAT_R30H121_PHYSICAL_OWNER||!runtimeOwner_.worker()||legacyBeginAttempted_||!authenticated_||!sessionReady()||!age)return false;
 if(link!=LINK_NRF_PRIMARY&&link!=LINK_LORA_BACKUP)return false;
 const bool backup=link==LINK_LORA_BACKUP;
 if(backup?!sx1262Ready_:!nrfReady_)return false;
 if(frame.sourceId!=BOAT_ID||frame.destinationId!=REMOTE_ID||
    frame.sessionEpoch!=sessionEpoch_||!frame.sequence||
    frame.sequence>=RadioSessionCounter::REKEY_AT||!radio_frameVerify(frame,key_))return false;
 // STOP ACK and STATUS have dedicated outboxes. Never bypass them.
 if(backup?(backupStopAck_.pending()||backupStatusOutbox_.pending()):
           (stopAck_.pending()||statusOutbox_.pending()))return false;
 using flytec_r30h120::TxPriority;
 TxPriority priority=TxPriority::None;
 RadioDecodedPayload decoded{};
 switch(frame.type){
  case RADIO_MSG_HEARTBEAT:
   if(!radio_decodeHeartbeatPayload(frame.payload,decoded))return false;
   priority=TxPriority::Heartbeat;break;
  case RADIO_MSG_ACK:
   if(!radio_decodeAckPayload(frame.payload,decoded)||decoded.commandType==CMD_EMERGENCY_STOP||decoded.commandId!=frame.commandId)return false;
   priority=TxPriority::Mission;break;
  case RADIO_MSG_MISSION_ACK:
  case RADIO_MSG_MISSION_FEEDBACK:priority=TxPriority::Mission;break;
  case RADIO_MSG_TELEMETRY:
   if(frame.payload[0]==flytec_time::SUBTYPE){
    uint64_t utc=0;
    if(backup||!flytec_time::decode(frame.payload,utc))return false;
    priority=TxPriority::Diagnostic;break;
   }
   if(!radio_decodeTelemetryPayload(frame.payload,decoded)||decoded.kind==RADIO_PAYLOAD_STATUS)return false;
   if(backup&&decoded.kind==RADIO_PAYLOAD_SAFETY_DIAGNOSTIC)return false;
   priority=decoded.kind==RADIO_PAYLOAD_SAFETY_DIAGNOSTIC?TxPriority::Diagnostic:TxPriority::Telemetry;break;
  default:return false;
 }
 RadioFrame outgoing=frame;
 outgoing.flags=backup?RADIO_FLAG_BACKUP_LINK:0;
 if(!radio_frameSeal(outgoing,key_))return false;
 auto &owner=backup?asyncBackupOwner_:asyncPrimaryOwner_;
 auto &tx=backup?asyncBackupTx_:asyncPrimaryTx_;
 return tx.submit(owner,outgoing,now,priority,age,[&](const RadioFrame &f){return radio_frameVerify(f,key_);},true);
}
void BoatDualRadioPort::completeAsyncApplication(ManagedLink link,const flytec_r30h118::Ticket &ticket,flytec_r30h121::TxState state,uint32_t now){
 if(!BOAT_R30H121_PHYSICAL_OWNER||legacyBeginAttempted_)return;
 if(link!=LINK_NRF_PRIMARY&&link!=LINK_LORA_BACKUP)return;
 const bool backup=link==LINK_LORA_BACKUP;
 auto &tx=backup?asyncBackupTx_:asyncPrimaryTx_;
 tx.complete(ticket,state,now,sessionStarted_?sessionEpoch_:uint8_t(0),
  [&](const RadioFrame &frame,flytec_r30h121::TxState result,uint32_t queuedAt){
   if(!backup&&frame.type==RADIO_MSG_TELEMETRY&&frame.payload[0]==8U){
    if(result==flytec_r30h121::TxState::LocalComplete)optionalDiagnostics_.sent(queuedAt);
    else optionalDiagnostics_.rejectedTx();
   }
   if(result!=flytec_r30h121::TxState::LocalComplete)return;
   if(!backup&&frame.type==RADIO_MSG_TELEMETRY&&frame.payload[0]==flytec_time::SUBTYPE){
    lastUtcTxMs_=queuedAt;utcEverSent_=true;
   }
   // Use the original queue time, never extend freshness by delayed polling.
   if(frame.type==RADIO_MSG_HEARTBEAT){
    if(backup)lastBackupHeartbeatTxMs_=queuedAt;
    else{lastNrfHeartbeatTxMs_=queuedAt;havePrimaryHeartbeatSent_=true;}
   }
   if(backup&&frame.type==RADIO_MSG_MISSION_FEEDBACK&&pendingBackupMissionFeedbackValid_){
    uint8_t expected[8]{};
    if(uint32_t(now-pendingBackupMissionFeedbackAtMs_)<=12000U&&
       uint32_t(queuedAt-pendingBackupMissionFeedbackAtMs_)<=12000U&&
       missionFeedbackEncode(pendingBackupMissionFeedback_,expected)&&
       memcmp(expected,frame.payload,8)==0)pendingBackupMissionFeedbackValid_=false;
   }
  });
}
bool BoatDualRadioPort::resetAsyncSession(uint8_t epoch){
 asyncPrimaryTx_.reset();asyncBackupTx_.reset();asyncTelemetry_.reset();
 const bool p=asyncPrimaryCycle_.resetSession(epoch);
 const bool b=asyncBackupCycle_.resetSession(epoch);
 return p&&b;
}
void BoatDualRadioPort::serviceAsyncOwner(ManagedLink link,uint32_t now,bool linkTrusted){
 if(!BOAT_R30H121_PHYSICAL_OWNER||!runtimeOwner_.worker()||legacyBeginAttempted_||!authenticated_)return;
 if(link!=LINK_NRF_PRIMARY&&link!=LINK_LORA_BACKUP)return;
 const bool backup=link==LINK_LORA_BACKUP;
 auto buildAck=[&](const boat_r30h18::StopAck &ack,RadioFrame &frame){
  if(!sessionReady()||ack.epoch!=sessionEpoch_)return false;
  radio_frameInit(frame,RADIO_MSG_ACK,BOAT_ID,REMOTE_ID,++txSequence_,ack.commandId,now,sessionEpoch_);
  frame.flags=backup?RADIO_FLAG_BACKUP_LINK:0;
  return radio_encodeAckPayload(ack.result,CMD_EMERGENCY_STOP,ack.commandId,ack.stopState,ack.interlocks,frame.payload)&&radio_frameSeal(frame,key_);
 };
 auto buildStatus=[&](const RadioDecodedPayload &status,RadioFrame &frame){
  if(!sessionReady()||status.kind!=RADIO_PAYLOAD_STATUS)return false;
  radio_frameInit(frame,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
  frame.flags=backup?RADIO_FLAG_BACKUP_LINK:0;
  return radio_encodeTelemetryPayload(status,frame.payload)&&radio_frameSeal(frame,key_);
 };
 auto verify=[&](const RadioFrame &frame){return radio_frameVerify(frame,key_);};
 auto result=[&](const flytec_r30h118::Ticket &ticket,flytec_r30h121::TxState state){
  completeAsyncApplication(link,ticket,state,now);
 };
 // Local handshake transmission completion cannot establish a session.
 auto hsResult=[](const flytec_r30h118::Ticket &,flytec_r30h120::HandshakeTx){};
 // One ordinary producer per free owner cycle. A heartbeat timestamp is
 // committed only by completeAsyncApplication, never by queue admission.
 auto ordinary=[&](flytec_r30h123::OwnerLink<RadioFrame>& owner){
  auto &tx=backup?asyncBackupTx_:asyncPrimaryTx_;
  if(!owner.feedbackReady()||tx.pending()||!sessionReady())return;
  if(backup&&pendingBackupMissionFeedbackValid_){
   const uint32_t elapsed=uint32_t(now-pendingBackupMissionFeedbackAtMs_);
   if(elapsed>=12000U)pendingBackupMissionFeedbackValid_=false;
   else if(backupFresh(now)&&(!lastBackupMissionFeedbackAttemptMs_||uint32_t(now-lastBackupMissionFeedbackAttemptMs_)>=250U)){
    lastBackupMissionFeedbackAttemptMs_=now;
    const auto feedback=pendingBackupMissionFeedback_;
    RadioFrame f{};
    radio_frameInit(f,RADIO_MSG_MISSION_FEEDBACK,BOAT_ID,REMOTE_ID,++txSequence_,feedback.transaction,now,sessionEpoch_);
    if(missionFeedbackEncode(feedback,f.payload)&&radio_frameSeal(f,key_))
     queueAsyncApplication(link,f,now,12000U-elapsed);
    return;
   }
  }
  const uint32_t last=backup?lastBackupHeartbeatTxMs_:lastNrfHeartbeatTxMs_;
  if(uint32_t(now-last)<(backup?3000U:100U)){
   if(backup||!primaryFresh(now))return;
   uint8_t payload[8]{};uint32_t remaining=0;
   if(asyncTelemetry_.take(now,payload,remaining)){
    RadioFrame f{};
    radio_frameInit(f,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
    memcpy(f.payload,payload,8);
    if(radio_frameSeal(f,key_))queueAsyncApplication(link,f,now,remaining);
    return;
   }
   // Lowest priority: primary-only optional data in the existing heartbeat gap.
   boat_r30g::Opportunity opportunity{};
   opportunity.authenticatedSession=sessionReady();opportunity.primaryFresh=primaryFresh(now);
   opportunity.txPending=!owner.feedbackReady();
   opportunity.commandQueued=commandQueue_.size()!=0||stopAck_.pending()||statusOutbox_.pending();
   opportunity.activeMission=controller_.status().transaction!=0;
   opportunity.failsafeOrLinkLoss=linkLossHold_||detachedReturn_||failsafeRthActive_;
   opportunity.recentCommand=lastControlRxValid_&&uint32_t(now-lastControlRxMs_)<boat_r30g::MIN_IDLE_AFTER_COMMAND_MS;
   opportunity.haveHeartbeat=havePrimaryHeartbeatSent_;opportunity.lastHeartbeatAtMs=lastNrfHeartbeatTxMs_;
   const uint32_t gap=uint32_t(now-lastNrfHeartbeatTxMs_);
   if(optionalDiagnostics_.take(opportunity,now,payload,&remaining)){
    const uint32_t window=boat_r30g::HEARTBEAT_GAP_END_MS+1U-gap;
    if(remaining>window)remaining=window;
    RadioFrame f{};
    radio_frameInit(f,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
    memcpy(f.payload,payload,8);
    if(!radio_frameSeal(f,key_)||!queueAsyncApplication(link,f,now,remaining))optionalDiagnostics_.rejectedTx();
    return;
   }
   if(!opportunity.authenticatedSession||!opportunity.primaryFresh||opportunity.txPending||
      opportunity.commandQueued||opportunity.activeMission||opportunity.failsafeOrLinkLoss||
      opportunity.recentCommand||!opportunity.haveHeartbeat||
      gap<boat_r30g::HEARTBEAT_GAP_BEGIN_MS||gap>boat_r30g::HEARTBEAT_GAP_END_MS||
      (utcEverSent_&&uint32_t(now-lastUtcTxMs_)<5000U))return;
   uint64_t utc=0;
   if(!flytec_utc::extrapolate(utcSource_,now,utc))return;
   remaining=flytec_time::SOURCE_MAX_AGE_MS+1U-uint32_t(now-utcSource_.localMs);
   const uint32_t window=boat_r30g::HEARTBEAT_GAP_END_MS+1U-gap;
   if(remaining>window)remaining=window;
   RadioFrame beacon{};
   radio_frameInit(beacon,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
   if(flytec_time::encode(utc,beacon.payload)&&radio_frameSeal(beacon,key_))
    queueAsyncApplication(link,beacon,now,remaining);
   return;
  }
  if(backup){
   if(uint32_t(now-lastBackupHeartbeatAttemptMs_)<250U)return;
   lastBackupHeartbeatAttemptMs_=now;
  }
  RadioFrame f{};
  radio_frameInit(f,RADIO_MSG_HEARTBEAT,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
  f.payload[0]=uint8_t(1U|(nrfReady_?2U:0U)|(sx1262Ready_?4U:0U));
  f.payload[1]=1;
  if(radio_frameSeal(f,key_))queueAsyncApplication(link,f,now,backup?6000U:250U);
 };
 if(!sessionReady()||!linkTrusted){
  if(backup)asyncBackupTx_.cancel(asyncBackupOwner_);
  else{asyncPrimaryTx_.cancel(asyncPrimaryOwner_);asyncTelemetry_.reset();optionalDiagnostics_.discard();}
 }
 if(backup)asyncBackupCycle_.step(now,12000U,12000U,sessionReady(),linkTrusted,buildAck,buildStatus,verify,result,hsResult,ordinary);
 else asyncPrimaryCycle_.step(now,2500U,2500U,sessionReady(),linkTrusted,buildAck,buildStatus,verify,result,hsResult,ordinary);
}
unsigned BoatDualRadioPort::drainAsyncRx(ManagedLink link,uint32_t now){
 // Deliberately unreachable until the legacy transmit boundary is migrated.
 // Do not dispatch into legacy PHY calls while a physical worker owns them.
 if(!BOAT_R30H121_PHYSICAL_OWNER||legacyBeginAttempted_||!authenticated_)return 0;
 if(link!=LINK_NRF_PRIMARY&&link!=LINK_LORA_BACKUP)return 0;
 auto &rx=link==LINK_NRF_PRIMARY?asyncPrimaryRx_:asyncBackupRx_;
 return rx.drain(now,100U,[&](){return sessionStarted_?sessionEpoch_:uint8_t(0);},
  [&](const RadioFrame &f){return radio_frameVerify(f,rootKey_);},
  [&](const RadioFrame &f){return radio_frameVerify(f,key_);},
  [&](const RadioFrame &f,uint32_t capturedAt){receiveCaptured(link,f,capturedAt);});
}
#endif
void BoatDualRadioPort::receiveCaptured(ManagedLink link,const RadioFrame &f,uint32_t capturedAt) {
 const uint32_t now=millis();
 if(!authenticated_||!radioLinkFlagMatches(f.flags,link==LINK_LORA_BACKUP)||f.sourceId!=REMOTE_ID||f.destinationId!=BOAT_ID)return;
 if(f.type>=SESSION_HELLO&&f.type<=SESSION_CONFIRM){
   if(f.sequence||f.commandId||f.sessionEpoch||!radio_frameVerify(f,rootKey_))return;
   SessionHandshakePacket reply{};bool newSession=false;
   if(!handshake_.boatReceive((SessionHandshakeMessage)f.type,f.payload,radioPlatformNonce(),rootKey_,radioPlatformHmac,now,true,reply,newSession))return;
   if(newSession){
     ++generation_;releaseIdSeen_=false;
     handshake_.frameKey(key_);sessionStarted_=true;lastSessionRx_=now;txSequence_=0;
     primarySeqGuard_.resetAfterHandshake();backupSeqGuard_.resetAfterHandshake();crossRadioSeqGuard_.resetAfterHandshake();
     lastNrfRxValid_=lastBackupRxValid_=false;haveAuthenticatedRx_=false;lastAnyAuthenticatedRxMs_=0;commandQueue_.clear();mission_.reset();dedupe_.clear();stickLease_.reset();radioWatchdog_.reset();
     // New authenticated epoch: discard any optional diagnostic from the
     // previous key/session and require a new successful primary heartbeat.
     optionalDiagnostics_.reset();lastControlRxValid_=false;
     backupStopAck_.reset();backupStatusOutbox_.reset();
     stopAck_.reset(); // new authenticated session invalidates prior ACK ID
     statusOutbox_.reset(); // no pre-handshake stopped status
     pendingBackupMissionFeedbackValid_=false;lastBackupMissionFeedbackAttemptMs_=0;
     havePrimaryHeartbeatSent_=false;
#if BOAT_R30H_LOCAL_TRACE_ENABLED
     havePrimaryTraceSample_=false;
     localTrace_.push(boat_r30h2::SESSION_ESTABLISHED,now,0,1,0);
#endif
     if(!detachedReturn_&&!linkLossHold_){abortMission();controller_.resetSession();}
   }
#if BOAT_R30H121_ENDPOINT_COMPILE
   if(BOAT_R30H121_PHYSICAL_OWNER&&newSession&&!resetAsyncSession(sessionEpoch_)){
    resetSession();return;
   }
#endif
   RadioFrame ack;radio_frameInit(ack,(RadioMessageType)reply.type,BOAT_ID,REMOTE_ID,0,0,now,0);
   memcpy(ack.payload,reply.payload,8);
#if BOAT_R30H121_ENDPOINT_COMPILE
   if(BOAT_R30H121_PHYSICAL_OWNER){
    ack.flags=link==LINK_LORA_BACKUP?RADIO_FLAG_BACKUP_LINK:0;
    if(radio_frameSeal(ack,rootKey_))queueAsyncHandshake(link,ack,now);
    return;
   }
#endif
    if(link==LINK_LORA_BACKUP){
      ack.flags=RADIO_FLAG_BACKUP_LINK;
      if(radio_frameSeal(ack,rootKey_)&&sx1262Ready_)admitBackupLegacy(ack,now);
    }else if(radio_frameSeal(ack,rootKey_)&&nrfReady_)admitPrimaryLegacy(ack);
   return;
 }
 if(!sessionStarted_||f.sessionEpoch!=sessionEpoch_||!radio_frameVerify(f,key_))return;
 if(f.type!=RADIO_MSG_HEARTBEAT&&f.type!=RADIO_MSG_CONTROL&&f.type!=RADIO_MSG_ACTION&&f.type!=RADIO_MSG_MISSION_PREPARE&&f.type!=RADIO_MSG_MISSION_CONTROL&&f.type!=RADIO_MSG_MISSION_STICKS)return;
 RadioSessionCounter &linkGuard=link==LINK_NRF_PRIMARY?primarySeqGuard_:backupSeqGuard_;
 if(!linkGuard.accept(f.sequence))return;
 // Conservative scheduling: even a later rejected authenticated control
 // packet can suppress optional TX; it never grants actuator authority.
 if(f.type!=RADIO_MSG_HEARTBEAT){
   lastControlRxValid_=true;lastControlRxMs_=now;
 }
 const bool crossSequenceAccepted=crossRadioSeqGuard_.accept(f.sequence);
 if(f.type==RADIO_MSG_HEARTBEAT){
   RadioDecodedPayload heartbeat{};if(!radio_decodeHeartbeatPayload(f.payload,heartbeat))return;
 }else if(!crossSequenceAccepted&&!(f.type==RADIO_MSG_ACTION&&f.payload[0]==CMD_EMERGENCY_STOP))return;
 bool &rxValid=link==LINK_NRF_PRIMARY?lastNrfRxValid_:lastBackupRxValid_;
 uint32_t &lastRx=link==LINK_NRF_PRIMARY?lastNrfRxMs_:lastBackupRxMs_;
 if(f.type==RADIO_MSG_HEARTBEAT){
   lastRx=capturedAt;lastSessionRx_=now;rxValid=true;noteAuthenticatedRx(capturedAt);
#if BOAT_R30H_LOCAL_TRACE_ENABLED
   const bool sample=link==LINK_LORA_BACKUP||!havePrimaryTraceSample_||
                     (uint32_t)(now-lastPrimaryTraceSampleAtMs_)>=1000U;
   if(sample){
     if(link==LINK_NRF_PRIMARY){havePrimaryTraceSample_=true;lastPrimaryTraceSampleAtMs_=now;}
     const uint8_t flags=(primaryFresh(now)?1U:0U)|(backupFresh(now)?2U:0U);
     localTrace_.push(link==LINK_NRF_PRIMARY?boat_r30h2::AUTH_PRIMARY_SAMPLE:
                     boat_r30h2::AUTH_BACKUP_SAMPLE,now,lastAnyAuthenticatedRxMs_,
                     (uint8_t)link,flags);
   }
#endif
   return;
 }
 // Detached Home return keeps the old plan private. Only a fresh neutral
 // primary stick sample may be observed, solely to qualify an explicit manual
 // takeover request; non-neutral sticks cannot steer or interrupt RTH.
 if(detachedReturn_&&f.type==RADIO_MSG_MISSION_PREPARE)return;
 if(f.type==RADIO_MSG_MISSION_STICKS){
   if(link!=LINK_NRF_PRIMARY||!crossSequenceAccepted)return;
   MissionStickInput input{};if(!missionStickDecode(f.payload,input)||f.commandId!=input.transaction)return;
   if(detachedReturn_&&(!input.neutral||input.left||input.right))return;
   if(stickLease_.receive(input,controller_.status().transaction,capturedAt,true)){lastRx=capturedAt;lastSessionRx_=now;rxValid=true;noteAuthenticatedRx(capturedAt);}
   return;
 }
 if(f.type==RADIO_MSG_MISSION_CONTROL){
   if(!crossSequenceAccepted)return;
   MissionControlRequest request{};if(!missionControlDecode(f.payload,request)||f.commandId!=request.transaction)return;
   // Slow E22 supports discrete autonomous PAUSE/CANCEL only; no manual takeover.
   if(link==LINK_LORA_BACKUP&&request.op!=MCTRL_PAUSE&&request.op!=MCTRL_CANCEL)return;
   lastRx=capturedAt;lastSessionRx_=now;rxValid=true;noteAuthenticatedRx(capturedAt);
   if(detachedReturn_&&request.op!=MCTRL_SWITCH_MANUAL){
    sendMissionFeedback({request.transaction,controller_.status().phase,(uint8_t)request.op,request.request,MC_REJECTED},now);return;
   }
   ExecInputs in{};const bool ready=missionInputs(in,now);
   MissionControlFeedback ack{};const bool newRequest=request.request>controller_.lastRequestId();
   if(controller_.request(request,in,now,true,ready,stickLease_.neutral(request.transaction,now),ack)){
     if(newRequest&&ack.result==MC_ACCEPTED&&request.op==MCTRL_START)commandQueue_.clear();
     if(newRequest&&ack.result==MC_ACCEPTED&&request.op!=MCTRL_START&&request.op!=MCTRL_RESUME)missionIntent_={INTENT_STOP,-1,0};
     if(newRequest&&ack.result==MC_ACCEPTED&&(request.op==MCTRL_TEMP_OVERRIDE||request.op==MCTRL_CANCEL||request.op==MCTRL_SWITCH_MANUAL))stickLease_.reset();
     if(newRequest&&ack.result==MC_ACCEPTED&&request.op==MCTRL_SWITCH_MANUAL&&detachedReturn_){
      detachedReturn_=false;failsafeRthActive_=false;linkLossHold_=false;missionIntent_={INTENT_STOP,-1,0};
     }
     sendMissionFeedback(ack,now);
   }else sendMissionFeedback({request.transaction,MC_IDLE,(uint8_t)request.op,request.request,MC_REJECTED},now);
   return;
 }
 if(f.type==RADIO_MSG_MISSION_PREPARE){
   if(!crossSequenceAccepted||!sessionReady())return;
   const uint16_t id=(uint16_t)f.payload[1]|((uint16_t)f.payload[2]<<8);
   const MissionOpcode op=(MissionOpcode)f.payload[0];
   if(!id||id!=f.commandId||op<MISSION_BEGIN||op>MISSION_CANCEL)return;
   lastRx=capturedAt;lastSessionRx_=now;rxValid=true;noteAuthenticatedRx(capturedAt);
   const bool stopped=missionGate_&&missionGate_();
   MissionReceive result=mission_.ingest(f.payload,now,true,stopped);
   if(result==MISSION_PREPARED){
    MissionPlan plan{};BoatHomePoint candidate={0,0,0},stored{};
    if(!mission_.getPrepared(plan,now)){mission_.reset();result=MISSION_REJECTED;}
    else{
     candidate.latE7=plan.homeLatE7;candidate.lonE7=plan.homeLonE7;
     const bool saved=homePointStore_.save&&homePointStore_.load&&
       boatHomeCoordinatesValid(candidate.latE7,candidate.lonE7)&&
       homePointStore_.save(candidate)&&homePointStore_.load(stored)&&
       boatHomePointValid(stored)&&stored.latE7==candidate.latE7&&stored.lonE7==candidate.lonE7;
     if(!saved||!controller_.stage(plan,now,true,stopped)){
      mission_.reset();result=MISSION_REJECTED;
     }else{stickLease_.reset();commandQueue_.clear();}
    }
   }
   if(result==MISSION_CANCELLED)abortMission();
   RadioFrame ack;radio_frameInit(ack,RADIO_MSG_MISSION_ACK,BOAT_ID,REMOTE_ID,++txSequence_,id,now,sessionEpoch_);
   if(missionAck_encode(id,op,result,ack.payload)&&radio_frameSeal(ack,key_))sendFrame(link,ack);
   return;
 }
 if(f.type==RADIO_MSG_HEARTBEAT || f.type==RADIO_MSG_TELEMETRY || f.type==RADIO_MSG_ACK || f.type==RADIO_MSG_LINK_PROBE) return;
 BoatCommand cmd;
 if(!radio_decodeCommand(f,REMOTE_ID,BOAT_ID,capturedAt,2500,link==LINK_LORA_BACKUP,cmd,key_)) return;
 const uint16_t id=(uint16_t)cmd.commandId;
 if(id==0) return;
 // An authenticated STOP invalidates any prepared plan before queue handling.
 lastRx=capturedAt;lastSessionRx_=now;rxValid=true;noteAuthenticatedRx(capturedAt);
 if(cmd.type==CMD_EMERGENCY_STOP){
#if BOAT_R30H_LOCAL_TRACE_ENABLED
   localTrace_.push(boat_r30h2::STOP_AUTH_RECEIVED,now,lastAnyAuthenticatedRxMs_,
                    (uint8_t)link,0,id);
#endif
   const bool wasDetached=detachedReturn_;
   detachedReturn_=false;failsafeRthActive_=false;linkLossHold_=false;
   abortMission();
   backupStatusOutbox_.reset();
   statusOutbox_.reset(); // discard any STATUS sampled before this STOP
#if BOAT_R30H_LOCAL_TRACE_ENABLED
   localTrace_.push(boat_r30h2::STOP_LOGICAL_INTENT,millis(),lastAnyAuthenticatedRxMs_,
                    (uint8_t)link,0,id);
#endif
   if(wasDetached)controller_.resetSession();
   mission_.reset();commandQueue_.clear();enqueue(cmd,link,false);
   if(sessionReady())sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
   return;
 }
 // A command frame rejected by the cross-radio sequence guard is accepted
 // only as an exact command-ID duplicate. This prevents replaying an old
 // sequence with a new command ID on the alternate radio.
 if(!crossSequenceAccepted) {
   const BoatCrossRadioCommandDecision decision=boatClassifyCrossRadioCommand(
       false,dedupe_.hasExecuted(f.sourceId,f.sessionEpoch,id),commandPending(f.sessionEpoch,id));
   if(decision==BOAT_CROSSRADIO_DUPLICATE_EXECUTED)
     sendCommandAck(link,cmd,RADIO_ACK_DUPLICATE,0,0,now);
   else if(decision==BOAT_CROSSRADIO_DUPLICATE_PENDING && cmd.type!=CMD_MANUAL_MOTORS)
     sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
   return;
 }
 const auto phase=controller_.status().phase;
 const bool homeRequest=cmd.type==CMD_SET_MODE&&cmd.mode==MODE_RETURN_HOME;
 if(detachedReturn_||(phase>=MC_PREPARED&&phase<=MC_RETURNING&&
    !(phase==MC_PREPARED&&cmd.type==CMD_CLEAR_STOP)&&!homeRequest)){if(cmd.type!=CMD_MANUAL_MOTORS)sendCommandAck(link,cmd,RADIO_ACK_REJECTED_SAFETY,2,0,now);return;}
 if(dedupe_.hasExecuted(f.sourceId,f.sessionEpoch,id)) {
   sendCommandAck(link,cmd,RADIO_ACK_DUPLICATE,0,0,now);return;
 }
 if(commandPending(f.sessionEpoch,id)) {
   if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
   return;
 }
 const bool release=cmd.type>=CMD_RELEASE_HOPPER_LEFT&&cmd.type<=CMD_RELEASE_HOOK_RIGHT;
 if(release&&releaseIdSeen_){const uint16_t delta=(uint16_t)(id-lastReleaseId_);if(!delta||delta>=0x8000U){sendCommandAck(link,cmd,RADIO_ACK_REJECTED_SAFETY,2,0,now);return;}}
 if(!enqueue(cmd,link,false)) {
   if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_BUSY,0,0,now);
   return;
 }
 if(release){lastReleaseId_=id;releaseIdSeen_=true;}
 // ACCEPTED means only queued for application review. It is not EXECUTED.
 if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
}
bool BoatDualRadioPort::commandPending(uint8_t epoch,uint16_t commandId) const {
 return commandQueue_.contains(epoch,commandId);
}
bool BoatDualRadioPort::sendCommandAck(ManagedLink link,const BoatCommand &cmd,uint8_t result,uint8_t stopState,uint8_t interlockFlags,uint32_t now) {
 if(!sessionReady())return false;
 // A primary STOP ACK must never be discarded just because an earlier
 // nRF packet still owns the asynchronous RF24 TX slot.
 if(link==LINK_LORA_BACKUP&&cmd.type==CMD_EMERGENCY_STOP)
  return backupStopAck_.offer((uint16_t)cmd.commandId,sessionEpoch_,result,stopState,interlockFlags,now);
 if(link==LINK_NRF_PRIMARY&&cmd.type==CMD_EMERGENCY_STOP)
  return stopAck_.offer((uint16_t)cmd.commandId,sessionEpoch_,result,
                        stopState,interlockFlags,now); // local queue, NOT delivered
 // Reject ordinary primary ACK while STOP/STATUS holds the slot BEFORE
 // allocating an authenticated sequence ID. Backup E22 is independent.
 if(link==LINK_NRF_PRIMARY&&!primaryOrdinaryTxReady())return false;
 RadioFrame f;
 radio_frameInit(f,RADIO_MSG_ACK,BOAT_ID,REMOTE_ID,++txSequence_,(uint16_t)cmd.commandId,now,sessionEpoch_);
 if(!radio_encodeAckPayload(result,(uint8_t)cmd.type,(uint16_t)cmd.commandId,stopState,interlockFlags,f.payload))return false;
 if(!radio_frameSeal(f,key_))return false;
 return sendFrame(link,f);
}
bool BoatDualRadioPort::enqueue(const BoatCommand &cmd,ManagedLink link,bool duplicate) {
 return commandQueue_.push(cmd,(uint8_t)link,duplicate,sessionEpoch_);
}
bool BoatDualRadioPort::takeCommand(BoatCommand &cmd,bool &duplicate,ManagedLink &link) {
 uint8_t linkId=0,epoch=0;
 while(commandQueue_.pop(cmd,duplicate,linkId,epoch)){
    if(!sessionStarted_||epoch!=sessionEpoch_)continue;
    if(cmd.type!=CMD_EMERGENCY_STOP&&millis()-cmd.createdAtMs>(cmd.type==CMD_MANUAL_MOTORS?250U:1500U))continue;
    link=(ManagedLink)linkId;return true;
 }
 return false;
}
bool BoatDualRadioPort::completeApplicationAction(const BoatCommand &cmd,uint32_t generation,bool success,uint8_t stopState,uint8_t interlocks,ManagedLink replyLink){
 if(generation!=generation_||!sessionReady())return false;
 if(success)dedupe_.markExecuted(REMOTE_ID,sessionEpoch_,(uint16_t)cmd.commandId);
 return sendCommandAck(replyLink,cmd,success?RADIO_ACK_EXECUTED:RADIO_ACK_REJECTED_SAFETY,stopState,interlocks,millis());
}
void BoatDualRadioPort::markActionExecuted(uint8_t sender,uint8_t epoch,uint16_t commandId) {
 dedupe_.markExecuted(sender,epoch,commandId);
}
bool BoatDualRadioPort::sendFrame(ManagedLink link,const RadioFrame &f) {
#if BOAT_R30H121_ENDPOINT_COMPILE
 if(runtimeOwner_.worker()){
  if(!BOAT_R30H121_PHYSICAL_OWNER)return false;
  // Non-STOP command ACKs and mission replies share the authenticated
  // ordinary lane. STOP ACK and STATUS always use dedicated outboxes.
  const uint32_t age=link==LINK_LORA_BACKUP?12000U:250U;
  return queueAsyncApplication(link,f,millis(),age);
 }
#endif
 if(!runtimeOwner_.legacy())return false; // no legacy PHY access in worker mode
 if(!sessionStarted_||!radio_frameVerify(f,key_)||!f.sequence||f.sequence>=RadioSessionCounter::REKEY_AT)return false;
 if(link==LINK_NRF_PRIMARY && nrfReady_){
  // Strict STOP-only reply priority over heartbeat, status, mission, UTC,
  // optional diagnostic and ordinary command ACK traffic. The STOP outbox
  // itself sends through admitPrimaryLegacy() after the RX poll.
  if(stopAck_.pending()||statusOutbox_.pending())return false;
  return admitPrimaryLegacy(f);
 }
 if(link==LINK_LORA_BACKUP && sx1262Ready_&&!backupStopAck_.pending()){RadioFrame backup=f;backup.flags=RADIO_FLAG_BACKUP_LINK;if(!radio_frameSeal(backup,key_))return false;return admitBackupLegacy(backup,millis());}
 return false;
}
bool BoatDualRadioPort::primaryOrdinaryTxReady() const {
#if BOAT_R30H121_ENDPOINT_COMPILE
 if(runtimeOwner_.worker())return BOAT_R30H121_PHYSICAL_OWNER&&sessionReady()&&nrfReady_&&
  asyncPrimaryOwner_.feedbackReady()&&!asyncPrimaryTx_.pending()&&!stopAck_.pending()&&!statusOutbox_.pending();
#endif
 if(!runtimeOwner_.legacy())return false;
 return boat_r30h21::ordinarySlotFree({
     sessionReady(),nrfReady_,primaryDriverPending(),
     stopAck_.pending(),statusOutbox_.pending()
 });
}
bool BoatDualRadioPort::sendHeartbeat(ManagedLink link,uint32_t now) {
 if(!sessionReady())return false;
 // R30H-21: a blocked 100-ms heartbeat may be retried every main loop.
 // Refuse it BEFORE ++txSequence_ so radio load cannot force early rekey.
 if(link==LINK_NRF_PRIMARY&&!primaryOrdinaryTxReady())return false;
 RadioFrame f;radio_frameInit(f,RADIO_MSG_HEARTBEAT,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
 f.payload[0]=(uint8_t)(1U|(nrfReady_?2U:0U)|(sx1262Ready_?4U:0U)); // alive + radio availability; STOP state unknown
 f.payload[1]=1; // protocol build-major placeholder; reserved payload bytes remain zero
 if(!radio_frameSeal(f,key_))return false;
 return sendFrame(link,f);
}
bool BoatDualRadioPort::primaryFresh(uint32_t now)const{return sessionStarted_ && nrfReady_ && lastNrfRxValid_ && (uint32_t)(now-lastNrfRxMs_)<=250;}
bool BoatDualRadioPort::backupFresh(uint32_t now)const{return sessionStarted_ && sx1262Ready_ && lastBackupRxValid_ && (uint32_t)(now-lastBackupRxMs_)<=6000;}
BoatFailsafeDecision BoatDualRadioPort::evaluateFailsafe(uint32_t now,bool mission,bool gnss,bool heading,bool battery,bool geofence)const {
 BoatLinkSafetyInputs in={primaryFresh(now),backupFresh(now),mission,gnss,heading,battery,geofence};
 return boatEvaluateLinkFailsafe(in);
}


// Keep synchronous admission semantics until asynchronous consumers are migrated.
bool BoatDualRadioPort::admitPrimaryLegacy(const RadioFrame &frame){return nrf_.sendFrame(frame);}
bool BoatDualRadioPort::admitBackupLegacy(const RadioFrame &frame,uint32_t now){return e22_.sendFrame(frame,now);}

// Legacy owner-only RX and health boundary.
bool BoatDualRadioPort::primaryDriverReady() const{return nrf_.ready();}
bool BoatDualRadioPort::primaryDriverPending() const{return nrf_.txPending();}
bool BoatDualRadioPort::primaryDriverSuccess() const{return nrf_.lastTxSuccess();}
bool BoatDualRadioPort::backupDriverReady() const{return e22_.ready();}
bool BoatDualRadioPort::backupDriverFault() const{return e22_.transportFault();}
bool BoatDualRadioPort::receivePrimaryLegacy(RadioFrame &f){return nrf_.receiveFrame(f);}
bool BoatDualRadioPort::receiveBackupLegacy(RadioFrame &f){return e22_.receiveFrame(f);}
