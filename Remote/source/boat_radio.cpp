#include "boat_radio.h"
#include "radio_payload_codec.h"
#include "radio_safety_diagnostic_v1.h"
#include "radio_ack_tracker.h"
#include "boat_telemetry.h"
#include "boat_state.h"
#include "radio_protocol.h"
#include "mission_wire.h"
#include "mission_control_client.h"
#include "mission_stick.h"
#include "joystick.h"
#include "radio_command_codec.h"
#include "radio_key_store.h"
#include "radio_session_platform.h"
#include "sc16is750_bridge.h"
#include "e22_uart_link.h"
#include "sc18is606_wire_io.h"
#include "mcp23017_ce_wire_io.h"
#include "sc18is606_nrf_link.h"
#include "r30h15_primary_stop_retry.h"
#include "r30h17_stop_confirmation_gate.h"
#include "radio_hw_config.h"
#include "remote_pin_assignments_r30h32.h"
#include "remote_r30h9_marker_config.h"
#include "r30h9_marker_contract.h"
// R30H-9: only read-only software events; do not turn Waveshare GPIO into probe output.
static_assert(REMOTE_R30H9_MARKER_GPIO == -1 && REMOTE_R30H9_GPIO_OUTPUT_SIGNED == 0,
              "Remote diagnostic marker must stay unwired and unsigned");
#include "radio_utc_time_v1.h"
#include <sys/time.h>
#include <Wire.h>
#include <esp_system.h>
#include <string.h>

// Provisional per-link FIFO dispatch budget; not a measured STOP deadline.
static constexpr unsigned RX_FRAME_BUDGET=4;
static DualRadioManager g_linkManager;
static flytec_time::Discipline g_gpsUtcClock;
#if REMOTE_R30H3_STOP_TRACE_ENABLED
static remote_r30h3::StopTrace g_stopTrace;
#endif
static RadioAckTracker g_ackTracker;
static SC16IS750Bridge g_uartBridge(Wire,remote_pins::SC16IS750_E22);
static E22UartLink g_lora(g_uartBridge,
    [](bool m0,bool m1,void*){ g_uartBridge.setE22Mode(m0,m1); },
    [](void*){ return g_uartBridge.e22AuxReady(); },
    [](bool released,void*){ g_uartBridge.setE22Reset(released); },
    nullptr, [](void*)->size_t { return static_cast<size_t>(g_uartBridge.availableForWrite()); });
// R30H-11: isolated I2C->SPI radio bridge, no ESP direct CE/CSN or TF SPI bus.
// Guarded against initialization until actual SC18IS606-EVB + nRF are measured.
static Sc18is606WireIo g_nrfBridgeIo(Wire);
static Mcp23017CeWireIo g_nrfCeWireIo(Wire);
static Mcp23017CePort g_nrfCe(g_nrfCeWireIo,REMOTE_NRF_CE_EXPANDER_ADDRESS);
static Sc18is606NrfLink g_nrf(g_nrfBridgeIo,g_nrfCe,
                              REMOTE_NRF_BRIDGE_I2C_ADDRESS,REMOTE_NRF_CHANNEL);
static bool g_loraConfigured=false;
static bool g_nrfReady=false;
static bool g_begun=false;
static constexpr uint8_t REMOTE_DEVICE_ID=0x01;
static constexpr uint8_t BOAT_DEVICE_ID=0x02;
static uint8_t g_sessionEpoch=1;
static uint16_t g_txSequence=0;
static uint32_t g_lastPrimaryHeartbeat=0;
static uint32_t g_lastBackupTxMs=0,g_lastBackupTxAttemptMs=0;
static bool g_backupTxSent=false,g_stopPending=false;
static BoatCommand g_pendingStop{};
static uint32_t g_backupStopStartedMs=0;
static constexpr uint32_t BACKUP_STOP_RETRY_LIFETIME_MS=15000U;
static flytec_r30h15::PrimaryStopRetry<BoatCommand> g_primaryStopRetry;
static flytec_r30h17::StopConfirmationGate g_stopConfirmation;
static flytec_r30h17::StopConfirmationGate g_backupStopConfirmation;
static bool g_backupStopConfirmed=false;
static uint8_t g_authKey[32],g_rootKey[32];
static bool g_rootReady=false;
static SessionHandshake g_session;
static uint32_t g_lastHandshakeTx=0;
static MissionUpload g_missionUpload;
static MissionControlClient g_missionControl;
static uint32_t g_lastStickTx=0;
static bool g_manualConfirmRequired=true,g_stopMustConfirm=true;
static uint32_t g_manualTransitionAt=0;
static MissionPlan g_missionPlan{};
static uint16_t g_missionId=0;
static uint32_t g_missionStarted=0;
static RadioSessionCounter g_boatSequenceGuard,g_boatPrimarySequenceGuard,g_boatBackupSequenceGuard;
static void resetSession(uint32_t now){
    g_session.reset();radioSession_wipe(g_authKey,32);
    g_gpsUtcClock.reset(); // no stale UTC authority across auth epochs
    g_manualConfirmRequired=true;g_stopMustConfirm=true;g_manualTransitionAt=now;
    g_txSequence=0;g_stopPending=false;g_primaryStopRetry.reset();g_stopConfirmation.reset();g_backupStopConfirmation.reset();g_backupStopConfirmed=false;g_missionId=0;g_missionUpload.fail();g_missionControl.reset();g_lastStickTx=now-50U;
    g_boatSequenceGuard.resetAfterHandshake();g_boatPrimarySequenceGuard.resetAfterHandshake();g_boatBackupSequenceGuard.resetAfterHandshake();
    g_ackTracker.reset(0);telemetryInit();
    boatState.emergencyStop=true;boatState.emergencyStopConfirmed=false;boatState.actuatorSafetyReady=false;
    g_linkManager.begin(now);g_linkManager.setTransportReady(LINK_NRF_PRIMARY,g_nrfReady);g_linkManager.setTransportReady(LINK_LORA_BACKUP,g_loraConfigured);
    g_lastHandshakeTx=now-500U;
}
static bool sendHandshake(const SessionHandshakePacket &packet,uint32_t now){
    if(!g_rootReady)return false;
    RadioFrame f;radio_frameInit(f,(RadioMessageType)packet.type,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,0,0,now,0);
    memcpy(f.payload,packet.payload,8);
    if(!radio_frameSeal(f,g_rootKey))return false;
    // Root-authenticated handshake may travel over either radio. E22 uses
    // its own link flag and seal; never equate UART admission with receipt.
    bool sent=false;
    if(g_nrfReady)sent=g_nrf.sendFrame(f);
    if(g_loraConfigured&&(!g_backupTxSent||now-g_lastBackupTxMs>=3000U)){
        f.flags=RADIO_FLAG_BACKUP_LINK;
        if(radio_frameSeal(f,g_rootKey)&&g_lora.sendFrame(f,now)){
            g_lastBackupTxMs=now;g_backupTxSent=true;sent=true;
        }
    }
    return sent;
}
static bool sessionReady(){return g_session.established()&&g_txSequence<RadioSessionCounter::REKEY_AT-1;}
static BoatRadioLink toPublicLink(ManagedLink link) {
    if(link==LINK_NRF_PRIMARY) return RADIO_LINK_NRF24_PRIMARY;
    if(link==LINK_LORA_BACKUP) return RADIO_LINK_LORA_BACKUP;
    return RADIO_LINK_NONE;
}
static BoatRadioState toPublicState(ManagedState state) {
    switch(state) {
        case LINK_PRIMARY:return RADIO_STATE_PRIMARY;
        case LINK_DEGRADED:return RADIO_STATE_DEGRADED;
        case LINK_BACKUP:return RADIO_STATE_BACKUP;
        case LINK_NO_LINK:return RADIO_STATE_NO_LINK;
        case LINK_RECOVERY:return RADIO_STATE_RECOVERY;
        default:return RADIO_STATE_DISCONNECTED;
    }
}

static bool radioSendTransport(ManagedLink link,const BoatCommand &command){
    if(!sessionReady())return false;
    const uint32_t now=millis();
    if(link==LINK_LORA_BACKUP){
        if(!g_loraConfigured)return false;
        // LoRa carries discrete authenticated commands at low rate; no manual
        // motor lease or STOP clearing until measured latency supports it.
        if(!DualRadioManager::backupCommandAllowed(command))return false;
        if(g_backupTxSent&&now-g_lastBackupTxMs<3000U){
            if(command.type==CMD_EMERGENCY_STOP){
              g_pendingStop=command;g_stopPending=true;
#if REMOTE_R30H3_STOP_TRACE_ENABLED
              g_stopTrace.record(remote_r30h3::E22_PACING_DEFERRED,now,
                                 (uint16_t)command.commandId,2);
#endif
              return true; // queued locally; NOT RF delivery
            }
            return false; // never queue obsolete movement
        }
    }else if(link!=LINK_NRF_PRIMARY||!g_nrfReady)return false;
    BoatCommand wire=command;wire.sequence=++g_txSequence;wire.createdAtMs=now;
    RadioFrame frame;
    if(!radio_encodeCommand(wire,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,g_sessionEpoch,frame,g_authKey))return false;
    if(link==LINK_NRF_PRIMARY){
        const bool admitted=g_nrf.sendFrame(frame);
#if REMOTE_R30H3_STOP_TRACE_ENABLED
        if(command.type==CMD_EMERGENCY_STOP)
            g_stopTrace.record(admitted?remote_r30h3::NRF_BUFFER_ADMITTED:
                              remote_r30h3::NRF_BUFFER_REJECTED,now,
                              (uint16_t)command.commandId,1);
#endif
        return admitted; // queue admission, NOT Boat STOP acknowledgement
    }
    frame.flags=RADIO_FLAG_BACKUP_LINK;
    if(!radio_frameSeal(frame,g_authKey))return false;
    if(g_lora.sendFrame(frame,now)){
      g_lastBackupTxMs=now;g_backupTxSent=true;
#if REMOTE_R30H3_STOP_TRACE_ENABLED
      if(command.type==CMD_EMERGENCY_STOP)
          g_stopTrace.record(remote_r30h3::E22_UART_BUFFER_ADMITTED,now,
                             (uint16_t)command.commandId,2);
#endif
      return true; // E22 UART accepted, NOT PHY airtime or receipt proof
    }
    if(command.type==CMD_EMERGENCY_STOP){
      g_pendingStop=command;g_stopPending=true;
#if REMOTE_R30H3_STOP_TRACE_ENABLED
      g_stopTrace.record(remote_r30h3::E22_UART_BUFFER_REJECTED_PENDING,now,
                         (uint16_t)command.commandId,2);
#endif
      return true; // still local pending, NOT delivered
    }
    return false;
}

bool boatRadio_begin() {
#if REMOTE_R30H3_STOP_TRACE_ENABLED
    g_stopTrace.reset();
#endif
    g_missionUpload=MissionUpload{};g_missionPlan={};g_missionId=0;
    g_linkManager.begin(millis());
    g_loraConfigured=false;g_nrfReady=false;g_begun=false;
    g_rootReady=radioKey_get(g_rootKey);
    g_sessionEpoch=1; // key, not legacy epoch, binds the session
    g_lastPrimaryHeartbeat=g_lastBackupTxMs=g_lastBackupTxAttemptMs=0;g_backupTxSent=false;
    resetSession(millis());g_missionUpload=MissionUpload{};

    // R30H-11: the selected software nRF adapter is SC18IS606 (separate
    // SPI), not GPIO6/15 or the TF bus. Four independent physical signoffs
    // remain 0 and prevent bridge initialization and all RF output.
    g_nrfReady=REMOTE_NRF_PIN_AUDIT_SIGNED &&
               REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED &&
               REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED &&
               g_nrf.begin(true);
    g_linkManager.setTransportReady(LINK_NRF_PRIMARY,g_nrfReady);

    // E22 is also locked until the exact 0x48 carrier/clock/TTL wiring, matched
    // module register readbacks, legal channel/antenna and *measured* airtime
    // budget are signed. The old 25ms estimate is NOT RF certification.
    if(REMOTE_E22_BRIDGE_AUDIT_SIGNED &&
       REMOTE_E22_RF_INTEROPERABILITY_VERIFIED &&
       REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED &&
       g_uartBridge.begin(9600) && g_lora.configureEU868TestProfile()) {
        g_loraConfigured=true;
        g_linkManager.setTransportReady(LINK_LORA_BACKUP,true);
    } else {
        g_linkManager.setTransportReady(LINK_LORA_BACKUP,false);
    }
    g_begun=g_nrfReady || g_loraConfigured;
    // No application TX until mutual nonce handshake completes.
    return g_begun; // configured hardware, not a connected/authenticated boat link
}

bool boatRadio_sendCommand(const BoatCommand &command) {
#if REMOTE_R30H3_STOP_TRACE_ENABLED
    if(command.type==CMD_EMERGENCY_STOP)
        g_stopTrace.record(remote_r30h3::STOP_BUTTON_REQUEST,millis(),
                           (uint16_t)command.commandId,0);
#endif
    if(command.type==CMD_EMERGENCY_STOP){
        g_missionControl.reset();g_stopMustConfirm=true;g_manualConfirmRequired=true;
        g_manualTransitionAt=millis();g_stopConfirmation.request(command.commandId);
        g_backupStopConfirmation.request(command.commandId);g_backupStopConfirmed=false;
    }
    if(command.type==CMD_EMERGENCY_STOP && (g_missionUpload.state()==UPLOAD_ACTIVE||g_missionUpload.state()==UPLOAD_PREPARED))g_missionUpload.fail();
    if(!g_begun) return false;
    if(command.type==CMD_MANUAL_MOTORS&&!boatRadio_manualModeConfirmed())return false;
    if(command.type!=CMD_EMERGENCY_STOP&&g_stopMustConfirm)return false;
    const uint32_t now=millis();
    bool queued=false;
    if(command.type==CMD_EMERGENCY_STOP&&sessionReady()){
        bool primary=false;
        if(g_nrfReady){
            // Keep the logical STOP until authenticated Boat STOP status.
            // A TX-buffer admission is never proof of physical execution.
            g_primaryStopRetry.queue(command,now);
            primary=radioSendTransport(LINK_NRF_PRIMARY,command);
            if(primary)g_primaryStopRetry.noteAdmitted(now);
            else primary=true; // accepted into bounded local STOP-only queue
        }
        if(g_loraConfigured){
            g_pendingStop=command;g_stopPending=true;g_backupStopStartedMs=now;
        }
        const bool backup=radioSendTransport(LINK_LORA_BACKUP,command);
        queued=primary||backup;
    }else queued=g_linkManager.send(command,radioSendTransport,now);
    if(queued && command.type!=CMD_MANUAL_MOTORS) {
        const uint16_t id=(uint16_t)(command.commandId&0xFFFFU);
        const uint8_t mode=(uint8_t)command.mode;
        const uint16_t timeout=(command.type>=CMD_RELEASE_HOPPER_LEFT && command.type<=CMD_RELEASE_HOOK_RIGHT)?3500U:2500U;
        if(id!=0) g_ackTracker.begin(id,(uint8_t)command.type,mode,now,timeout);
    }
    return queued;
}

static bool missionPreparationSafe(){
    const uint32_t now=millis();
    const bool batteryFresh=telemetry.batteryPercentValid&&
        (uint32_t)(now-telemetry.batteryPercentReceivedAtMs)<=1500U&&
        telemetry.batteryVoltageValid&&(uint32_t)(now-telemetry.batteryReceivedAtMs)<=5000U;
    return batteryFresh&&telemetry.batteryPercent>20&&!boatState.lowBattery&&!boatState.criticalBattery&&
           g_begun && g_nrfReady && g_session.established() &&
           g_linkManager.active()==LINK_NRF_PRIMARY && g_linkManager.primaryHealthy(millis()) &&
           boatState.radioConnected && boatState.mode==MODE_MANUAL &&
           boatState.emergencyStop && boatState.emergencyStopConfirmed &&
           !boatState.motorLeftActive && !boatState.motorRightActive &&
           !boatState.failsafeActive && sessionReady();
}
static bool sendMissionPreparation(const uint8_t payload[8]){
    if(!g_nrfReady||!sessionReady())return false;
    RadioFrame frame;
    const uint16_t id=(uint16_t)payload[1]|((uint16_t)payload[2]<<8);
    radio_frameInit(frame,RADIO_MSG_MISSION_PREPARE,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,++g_txSequence,id,millis(),g_sessionEpoch);
    memcpy(frame.payload,payload,8);
    return radio_frameSeal(frame,g_authKey)&&g_nrf.sendFrame(frame);
}
bool boatRadio_prepareMission(MissionPlan plan){
    if(!missionPreparationSafe()||boatRadio_missionLocked())return false;
    if(++g_missionId==0)++g_missionId;
    plan.transaction=g_missionId;
    const uint32_t now=millis();
    if(!g_missionUpload.begin(plan,now,true))return false;
    g_missionControl.reset();g_missionPlan=plan;g_missionStarted=now;return true;
}
MissionUploadState boatRadio_missionUploadState(){return g_missionUpload.state();}
static bool missionPrimaryReady(){
 return sessionReady()&&g_nrfReady&&g_linkManager.active()==LINK_NRF_PRIMARY&&g_linkManager.primaryHealthy(millis());
}
MissionControlPhase boatRadio_missionPhase(){return g_missionControl.phase();}
bool boatRadio_missionFeedbackFresh(){return missionPrimaryReady()&&g_missionControl.fresh(millis());}
bool boatRadio_missionControlPending(){return g_missionControl.pending();}
bool boatRadio_missionControlUnknown(){return g_missionControl.unknown();}
MissionControlResult boatRadio_missionControlResult(){return g_missionControl.result();}
bool boatRadio_missionLocked(){return g_missionUpload.state()==UPLOAD_ACTIVE||g_missionUpload.state()==UPLOAD_PREPARED||g_missionControl.locked();}
bool boatRadio_backupStopConfirmed(){return g_backupStopConfirmed&&sessionReady()&&g_linkManager.backupHealthy(millis());}
bool boatRadio_stopConfirmationPending(){return g_stopMustConfirm;}
bool boatRadio_manualModeConfirmed(){return !g_manualConfirmRequired&&!g_stopMustConfirm&&!boatRadio_missionLocked()&&telemetry.statusValid&&telemetry.statusPrimary&&millis()-telemetry.statusReceivedAtMs<1000U&&telemetry.reportedMode==MODE_MANUAL;}
bool boatRadio_missionControl(MissionControlOp op){
 if(!missionPrimaryReady()||!g_missionControl.transaction())return false;
 if(op!=MCTRL_CANCEL){
  if(g_stopMustConfirm||!boatRadio_missionFeedbackFresh()||!boatState.emergencyStopConfirmed||boatState.emergencyStop||boatState.failsafeActive||!boatState.actuatorSafetyReady)return false;
  if(op==MCTRL_START||op==MCTRL_RESUME){const auto j=joystick_getSnapshot();if(!j.readingsValid||!j.calibrationValid||j.calibrationMode||!j.neutral)return false;}
  if(op==MCTRL_START&&g_missionControl.phase()!=MC_PREPARED)return false;
 }
 const bool queued=g_missionControl.request(op,millis());
 if(queued){g_manualConfirmRequired=true;g_manualTransitionAt=millis();}
 return queued;
}
bool boatRadio_sendMissionSticks(int16_t left,int16_t right,bool neutral){
 if(!boatRadio_missionFeedbackFresh()||!g_missionControl.transaction())return false;
 const auto phase=g_missionControl.phase();
 if(phase!=MC_PREPARED&&phase!=MC_PAUSED&&phase!=MC_OVERRIDE_NEUTRAL&&phase!=MC_OVERRIDE_ACTIVE)return false;
 // Before confirmed override, send neutral evidence only; never movement.
 if(phase!=MC_OVERRIDE_ACTIVE){left=right=0;}
 else if(g_stopMustConfirm||boatState.emergencyStop||!boatState.emergencyStopConfirmed||boatState.failsafeActive||!boatState.actuatorSafetyReady)return false;
 const uint32_t now=millis();if(now-g_lastStickTx<50U)return true;
 MissionStickInput input={g_missionControl.transaction(),left,right,neutral};
 RadioFrame frame;radio_frameInit(frame,RADIO_MSG_MISSION_STICKS,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,++g_txSequence,input.transaction,now,g_sessionEpoch);
 if(!missionStickEncode(input,frame.payload)||!radio_frameSeal(frame,g_authKey))return false;
 g_lastStickTx=now;return g_nrf.sendFrame(frame);
}
void boatRadio_cancelMissionPreparation(){
    if(g_missionControl.transaction()){boatRadio_missionControl(MCTRL_CANCEL);return;}
    if(g_missionUpload.state()!=UPLOAD_ACTIVE&&g_missionUpload.state()!=UPLOAD_PREPARED)return;
    uint8_t payload[8];
    if(missionPlan_encode(g_missionPlan,MISSION_CANCEL,payload))sendMissionPreparation(payload);
    g_missionUpload.fail(); // cancellation unconfirmed; receiver preparation expires independently
}
static bool dispatchValidatedPayload(ManagedLink link,const RadioFrame &frame,uint32_t now) {
    RadioDecodedPayload decoded{};
    switch(frame.type) {
        case RADIO_MSG_MISSION_FEEDBACK:{
            MissionControlFeedback feedback{};
            if(!missionFeedbackDecode(frame.payload,feedback)||feedback.transaction!=frame.commandId)return false;
            const auto old=g_missionControl.phase();
            if(!g_missionControl.observe(feedback,now,true))return false;
            if(old!=feedback.phase&&(feedback.phase==MC_MANUAL_REQUESTED||feedback.phase==MC_COMPLETE||feedback.phase==MC_ABORTED||feedback.phase==MC_IDLE)){
                g_manualConfirmRequired=true;g_manualTransitionAt=now;
            }
            return true;
        }
        case RADIO_MSG_MISSION_ACK:{
            uint16_t id;MissionOpcode op;MissionReceive result;
            if(link!=LINK_NRF_PRIMARY||!missionAck_decode(frame.payload,id,op,result)||id!=frame.commandId)return false;
            if(g_missionUpload.acknowledge(id,op,result,now,true)&&g_missionUpload.state()==UPLOAD_PREPARED){g_missionControl.bind(id);g_missionUpload=MissionUpload{};}
            return true;
        }
        case RADIO_MSG_HEARTBEAT:
            return radio_decodeHeartbeatPayload(frame.payload,decoded);
        case RADIO_MSG_ACK:
            if(!radio_decodeAckPayload(frame.payload,decoded)) return false;
            // Authenticated, session-bound ACKs may update only a matching command record.
            if(g_ackTracker.observe(decoded,link==LINK_NRF_PRIMARY?RADIO_ACK_LINK_PRIMARY:RADIO_ACK_LINK_BACKUP,
                                    frame.sessionEpoch,now) &&
               decoded.commandType==CMD_EMERGENCY_STOP){
                // ACCEPTED does not prove physical STOP; it only binds the
                // next sequenced primary status to this logical request ID.
                const bool accepted=decoded.ackResult==RADIO_ACK_ACCEPTED||
                                    decoded.ackResult==RADIO_ACK_EXECUTED;
                if(link==LINK_LORA_BACKUP)g_backupStopConfirmation.acceptedAck(decoded.commandId,accepted);
                g_stopConfirmation.acceptedAck(decoded.commandId,accepted);
            }
            return true;
        case RADIO_MSG_TELEMETRY:
            if(frame.payload[0]==flytec_time::SUBTYPE){
                // HMAC + session/epoch + sequence + nRF link checked by caller.
                // Time NEVER refreshes control-link freshness, lease, stopState,
                // battery, mission or other Boat telemetry.
                if(link!=LINK_NRF_PRIMARY)return false;
                uint64_t utcMs=0;
                return flytec_time::decode(frame.payload,utcMs) &&
                       g_gpsUtcClock.observe(utcMs,now);
            }
            if(!radio_decodeTelemetryPayload(frame.payload,decoded)) return false;
            if(!telemetryApplyRadioPayload(decoded,link==LINK_NRF_PRIMARY,now))return false;
            if(link==LINK_LORA_BACKUP&&decoded.kind==RADIO_PAYLOAD_STATUS){
                g_backupStopConfirmed=g_backupStopConfirmation.armed()&&g_backupStopConfirmation.allowsStopStatus(true,decoded.stopState==1);
                if(g_backupStopConfirmed)g_stopPending=false; // does not clear primary movement interlock
            }
            if(link==LINK_NRF_PRIMARY&&decoded.kind==RADIO_PAYLOAD_STATUS){
                if(g_stopConfirmation.allowsStopStatus(true,decoded.stopState==1)){
                    g_stopMustConfirm=false;
                    // After HMAC/session/sequence validation and telemetryApply,
                    // STOP is confirmed logically at Boat. Not physical current-zero.
                    g_primaryStopRetry.reset();
                    g_stopPending=false;
                    g_stopConfirmation.reset();
                }
                if(!g_stopMustConfirm&&!boatRadio_missionLocked()&&decoded.boatMode==MODE_MANUAL&&!(decoded.safetyFlags&6U)&&((int32_t)(now-g_manualTransitionAt)>0))g_manualConfirmRequired=false;
            }
            return true;
        case RADIO_MSG_LINK_PROBE:
            return true;
        default:
            return false;
    }
}
static bool payloadBytesZero(const uint8_t *p,uint8_t start,uint8_t end) {
    for(uint8_t i=start;i<=end;i++)if(p[i]!=0)return false;
    return true;
}
static void validateInboundFrame(ManagedLink link,const RadioFrame &frame){
    const uint32_t now=millis();
    if(!radioLinkFlagMatches(frame.flags,link==LINK_LORA_BACKUP)||frame.sourceId!=BOAT_DEVICE_ID||frame.destinationId!=REMOTE_DEVICE_ID)return;
    if(frame.type>=SESSION_HELLO&&frame.type<=SESSION_CONFIRM){
        if(!g_rootReady||frame.sequence||frame.commandId||frame.sessionEpoch||!radio_frameVerify(frame,g_rootKey))return;
        if(g_session.remoteReceive((SessionHandshakeMessage)frame.type,frame.payload,g_rootKey,radioPlatformHmac,now,true)&&g_session.established()){
            g_session.frameKey(g_authKey);g_ackTracker.reset(g_sessionEpoch);
        }
        return;
    }
    if(!g_session.established()||frame.sessionEpoch!=g_sessionEpoch||!radio_frameVerify(frame,g_authKey))return;
    if(frame.type!=RADIO_MSG_HEARTBEAT&&frame.type!=RADIO_MSG_TELEMETRY&&frame.type!=RADIO_MSG_ACK&&frame.type!=RADIO_MSG_LINK_PROBE&&frame.type!=RADIO_MSG_MISSION_ACK&&frame.type!=RADIO_MSG_MISSION_FEEDBACK)return;
    if(frame.type==RADIO_MSG_LINK_PROBE&&!payloadBytesZero(frame.payload,0,7))return;
    RadioSessionCounter &guard=link==LINK_NRF_PRIMARY?g_boatPrimarySequenceGuard:g_boatBackupSequenceGuard;
    if(!guard.accept(frame.sequence))return;
    const bool cross=g_boatSequenceGuard.accept(frame.sequence);
    if(!cross&&frame.type!=RADIO_MSG_HEARTBEAT)return;
    if(!dispatchValidatedPayload(link,frame,now))return;
    // R30F authenticated diagnostic is informational. A diagnostic-only
    // packet may NEVER extend authenticated-control link freshness, which
    // would otherwise indirectly authorize manual sticks or STOP clearing.
    if(frame.type==RADIO_MSG_TELEMETRY &&
       (frame.payload[0]==radio_r30f::SUBTYPE ||
        frame.payload[0]==flytec_time::SUBTYPE))return;
    g_linkManager.reportValidRx(link,true,now);
}

void boatRadio_update() {
    if(!g_begun) return;
    const uint32_t now=millis();
    // Link inactivity is supervised by DualRadioManager; do not wipe the
    // authenticated session merely because both RF paths were quiet. This
    // allows the backup E22 path to restore the same session after an outage.
    // Sequence-limit rekey still uses the existing primary handshake.
    if(g_session.established()&&!sessionReady())resetSession(now);
    if(g_session.timeout(now))resetSession(now);
    if(g_rootReady&&(g_nrfReady||g_loraConfigured)&&!g_session.established()&&now-g_lastHandshakeTx>=500U){
        g_lastHandshakeTx=now;g_session.startRemote(radioPlatformNonce(),now);
        SessionHandshakePacket packet{};if(g_session.remotePacket(packet))sendHandshake(packet,now);
    }
    // R30H-15: release TX completions before heartbeat can claim the slot.
    // No RF on default-locked builds: g_nrfReady remains false until signoff.
    if(g_nrfReady) {
        g_nrf.update();
        if(!g_nrf.ready()){
            // I2C/bridge fault revokes the primary immediately. Do NOT
            // keep stale readiness or replay queued manual/release commands.
            g_nrfReady=false;
            g_linkManager.setTransportReady(LINK_NRF_PRIMARY,false);
        }
        RadioFrame frame;
        for(unsigned i=0;g_nrfReady&&i<RX_FRAME_BUDGET&&g_nrf.receiveFrame(frame);++i) validateInboundFrame(LINK_NRF_PRIMARY,frame);
    }
    if(g_loraConfigured) {
        g_lora.update();
        if(g_lora.transportFault()){
            // Revoke stale backup health; do not clear the STOP confirmation gate.
            g_loraConfigured=false;g_stopPending=false;g_backupStopConfirmed=false;g_backupStopConfirmation.reset();
            g_linkManager.setTransportReady(LINK_LORA_BACKUP,false);
            g_linkManager.reportFailure(LINK_LORA_BACKUP);
        }else{
            RadioFrame frame;
            for(unsigned i=0;i<RX_FRAME_BUDGET&&g_lora.receiveFrame(frame);++i) validateInboundFrame(LINK_LORA_BACKUP,frame);
        }
    }
    if(g_stopPending&&(!sessionReady()||!g_loraConfigured||
       (uint32_t)(millis()-g_backupStopStartedMs)>=BACKUP_STOP_RETRY_LIFETIME_MS))
        g_stopPending=false; // expiry never clears the confirmation interlock
    if(g_primaryStopRetry.pending()){
        if(!sessionReady()||!g_nrfReady||g_primaryStopRetry.expired(millis())){
            g_primaryStopRetry.reset(); // never replay after timeout/epoch loss
        }else if(g_primaryStopRetry.mayAttempt(sessionReady(),g_nrfReady,
                                               g_nrf.txPending(),millis())){
            const BoatCommand pending=g_primaryStopRetry.command();
            if(radioSendTransport(LINK_NRF_PRIMARY,pending))
                g_primaryStopRetry.noteAdmitted(millis()); // await Boat STOP status
        }
    }
    if(sessionReady()){
        if(g_primaryStopRetry.maySendPeriodic(g_nrfReady,g_nrf.txPending())&&
           now-g_lastPrimaryHeartbeat>=100U){
            g_lastPrimaryHeartbeat=now;RadioFrame hb;radio_frameInit(hb,RADIO_MSG_HEARTBEAT,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,++g_txSequence,0,now,g_sessionEpoch);
            hb.payload[0]=(uint8_t)(1U|(g_nrfReady?2U:0U)|(g_loraConfigured?4U:0U)|(boatState.emergencyStopConfirmed&&boatState.emergencyStop?8U:0U));hb.payload[1]=1;
            if(radio_frameSeal(hb,g_authKey))g_nrf.sendFrame(hb);
        }
        // STOP is handled first. Give queued PAUSE/CANCEL the next E22 slot
        // before the periodic heartbeat consumes that slot.
    MissionControlRequest request{};
    const bool primary=missionPrimaryReady();
    const bool backup=!primary&&g_loraConfigured&&g_linkManager.backupHealthy(now);
    const bool backupSlot=!g_backupTxSent||(uint32_t)(now-g_lastBackupTxMs)>=3000U;
    g_missionControl.expire(now,backup);
    if(sessionReady()&&!g_stopPending&&(primary||(backup&&backupSlot))&&
       g_missionControl.due(now,request,backup)){
        // E22 backup is limited to discrete PAUSE/CANCEL; do not consume a
        // retry for unsupported commands or when either transport is busy.
        if(primary||request.op==MCTRL_PAUSE||request.op==MCTRL_CANCEL){
            const bool transportFree=primary?!g_nrf.txPending():g_lora.ready();
            if(transportFree){
                RadioFrame f;
                radio_frameInit(f,RADIO_MSG_MISSION_CONTROL,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,
                                ++g_txSequence,request.transaction,now,g_sessionEpoch);
                if(missionControlEncode(request,f.payload)){
                    if(backup)f.flags=RADIO_FLAG_BACKUP_LINK;
                    if(radio_frameSeal(f,g_authKey)){
                        const bool sent=primary?g_nrf.sendFrame(f):g_lora.sendFrame(f,now);
                        if(sent){
                            g_missionControl.admitted(now,backup);
                            if(backup){g_lastBackupTxMs=now;g_backupTxSent=true;}
                        }
                    }
                }
            }
        }
    }
        if(sessionReady()&&g_loraConfigured&&(!g_backupTxSent||now-g_lastBackupTxMs>=3000U)&&now-g_lastBackupTxAttemptMs>=250U){
            g_lastBackupTxAttemptMs=now;
            if(g_stopPending){
                BoatCommand pending=g_pendingStop; // retain through UART admission
                radioSendTransport(LINK_LORA_BACKUP,pending);
            }else{
                RadioFrame hb;radio_frameInit(hb,RADIO_MSG_HEARTBEAT,REMOTE_DEVICE_ID,BOAT_DEVICE_ID,++g_txSequence,0,now,g_sessionEpoch);
                hb.payload[0]=(uint8_t)(1U|(g_nrfReady?2U:0U)|(g_loraConfigured?4U:0U)|(boatState.emergencyStopConfirmed&&boatState.emergencyStop?8U:0U));hb.payload[1]=1;
                hb.flags=RADIO_FLAG_BACKUP_LINK;
                if(radio_frameSeal(hb,g_authKey)&&g_lora.sendFrame(hb,now)){g_lastBackupTxMs=now;g_backupTxSent=true;}
            }
        }
    }
    g_ackTracker.update(now);
    g_linkManager.tick(millis());
    // Update ESP32 CLOCK_REALTIME only after two consistent authenticated
    // Boat GPS UTC time samples; never touch millis() or safety timers.
    const uint32_t utcLocalNow=millis();
    uint64_t unixMs=0;
    if(sessionReady()&&g_linkManager.primaryHealthy(utcLocalNow)&&
       g_gpsUtcClock.shouldSetRtc(utcLocalNow)&&
       g_gpsUtcClock.utcNow(utcLocalNow,unixMs)){
        timeval tv{};tv.tv_sec=(time_t)(unixMs/1000ULL);
        tv.tv_usec=(suseconds_t)((unixMs%1000ULL)*1000ULL);
        if(settimeofday(&tv,nullptr)==0)g_gpsUtcClock.markRtcSet(utcLocalNow);
    }
    const bool safe=missionPreparationSafe();
    if(g_missionUpload.state()==UPLOAD_ACTIVE&&(!safe||now-g_missionStarted>=9000U))boatRadio_cancelMissionPreparation();
    uint8_t missionPayload[8];
    if(g_missionUpload.next(now,safe,missionPayload))sendMissionPreparation(missionPayload);

}

bool boatRadio_utcNow(uint64_t &unixMs){
    const uint32_t at=millis();
    return sessionReady()&&g_linkManager.primaryHealthy(at)&&
           g_gpsUtcClock.utcNow(at,unixMs);
}
flytec_r30h6::ClockReport boatRadio_utcQuality(){
    // Display/status path only; a stale/unauthenticated primary link makes
    // the quality INVALID, without touching STOP, radio freshness or RTH.
    const uint32_t now=millis();
    return flytec_r30h6::report(g_gpsUtcClock,now,false,
              sessionReady()&&g_linkManager.primaryHealthy(now));
}
bool boatRadio_isConnected(){return g_session.established()&&g_linkManager.connected(millis());}
BoatRadioLink boatRadio_getActiveLink(){return toPublicLink(g_linkManager.active());}
BoatRadioState boatRadio_getState(){return toPublicState(g_linkManager.state());}
bool boatRadio_primaryConnected(){return g_linkManager.primaryHealthy(millis());}
bool boatRadio_backupConnected(){return g_linkManager.backupHealthy(millis());}
bool boatRadio_getCommandAck(uint16_t commandId,RadioCommandAckRecord &record){return g_ackTracker.get(commandId,record);}
uint8_t boatRadio_pendingCommandAcks(){return g_ackTracker.pendingCount();}
void boatRadio_setTransport(RadioSendFn transport){(void)transport;} // fixed in this adapter
void boatRadio_setReady(ManagedLink link,bool ready){g_linkManager.setTransportReady(link,ready);}
void boatRadio_reportValidPacket(ManagedLink link,bool authenticated,uint32_t receivedAtMs){(void)link;(void)authenticated;(void)receivedAtMs;} // only verified dispatch updates health
void boatRadio_reportFailure(ManagedLink link){g_linkManager.reportFailure(link);}

bool boatRadio_takeR30H3StopTrace(remote_r30h3::Record &out){
#if REMOTE_R30H3_STOP_TRACE_ENABLED
    return g_stopTrace.take(out);
#else
    (void)out;
    return false;
#endif
}
uint32_t boatRadio_r30h3TraceOverflows(){
#if REMOTE_R30H3_STOP_TRACE_ENABLED
    return g_stopTrace.overflows();
#else
    return 0;
#endif
}
