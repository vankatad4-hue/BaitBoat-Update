#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "nrf24_link.h"
#include "e22_uart_board_port.h"
#include "boat_radio_hw_config.h"
#include "boat_r30h9_marker_config.h"
#include "r30h9_marker_contract.h"
// R30H-9: marker commissioning impossible in diagnostic firmware.
static_assert(BOAT_R30H9_MARKER_GPIO == -1 && BOAT_R30H9_GPIO_OUTPUT_SIGNED == 0,
              "Boat marker GPIO must remain electrically disconnected and unsigned");
#include "boat_r30h2_local_trace.h"
#include "gnss_utc_sync.h"
#include "radio_utc_time_v1.h"
#include "radio_command_codec.h"
#include "radio_key_store.h"
#include "boat_radio_safety.h"
#include "boat_home_point_record.h"
#include "radio_payload_encoder.h"
#include "boat_r30g_optional_diag_budget.h"
#include "boat_r30h18_stop_ack_outbox.h"
#include "boat_r30h19_status_outbox.h"
#include "boat_r30h21_primary_tx_budget.h"
#include "boat_command_queue.h"
#include "mission_wire.h"
#include "mission_controller.h"
#include "mission_stick.h"
#ifndef BOAT_R30H124W3_NEUTRAL_RECEIPT
#define BOAT_R30H124W3_NEUTRAL_RECEIPT 0
#endif
#if BOAT_R30H124W3_NEUTRAL_RECEIPT
#include "boat_neutral_receipt.h"
#endif
#include "radio_session_platform.h"
#include "boat_session_recovery.h"
#include "boat_handshake_reply.h"
#include "boat_radio_owner.h"

// R30H-121E: compile-only application endpoint seam; never starts a PHY task.
#ifndef BOAT_R30H121_ENDPOINT_COMPILE
#define BOAT_R30H121_ENDPOINT_COMPILE 0
#endif
#if BOAT_R30H121_ENDPOINT_COMPILE
#include "boat_radio_services.h"
#include "boat_radio_physical_worker.h"
#endif

// Link values match the Remote manager and queue records.
enum ManagedLink : uint8_t { LINK_NONE=0, LINK_NRF_PRIMARY=1, LINK_LORA_BACKUP=2 };

// All physical GPIO values must be audited/frozen for the exact ESP32-S3 SKU.
// Default -1 macros intentionally keep radio hardware disabled.
#ifndef BOAT_NRF_CE_PIN
#define BOAT_NRF_CE_PIN (-1)
#endif
#ifndef BOAT_NRF_CSN_PIN
#define BOAT_NRF_CSN_PIN (-1)
#endif

// No production worker selection until async consumers and shutdown are verified.
#ifndef BOAT_R30H121_PHYSICAL_OWNER
#define BOAT_R30H121_PHYSICAL_OWNER 0
#endif
static_assert(BOAT_R30H121_PHYSICAL_OWNER == 0, "Physical ownership migration not commissioned");

class BoatDualRadioPort {
public:
    BoatDualRadioPort();
    bool begin();
    // Application-owned verification of physical STOP/stationary state.
    void setMissionPreparationGate(bool (*gate)()) { missionGate_=gate; mission_.reset(); }
    bool preparedMission(MissionPlan &out,uint32_t now) const { return missionGate_ && missionGate_() && primaryFresh(now) && mission_.getPrepared(out,now); }
    // Null provider denies execution; diagnostic sketch never registers one.
    void setMissionInputs(bool (*input)(ExecInputs &)){missionInput_=input;controller_.emergencyAbort();stickLease_.reset();}
    ExecOutput missionIntent()const{return missionIntent_;}
    MissionControlPhase missionPhase()const{return controller_.status().phase;}
    uint16_t missionTransaction()const{return controller_.status().transaction;}
    bool detachedReturnActive()const{return detachedReturn_;}
    MissionNavigationResult missionNavigation()const{ExecInputs in{};return missionInputs(in,millis())?controller_.navigation(in):MissionNavigationResult{};}
    void localEmergencyStop(){const bool wasDetached=detachedReturn_;abortMission();detachedReturn_=false;failsafeRthActive_=false;linkLossHold_=false;if(wasDetached)controller_.resetSession();commandQueue_.clear();mission_.reset();}
    bool confirmManual(){ExecInputs in{};return missionInputs(in,millis())&&controller_.confirmManual(in);}
    // Authenticated command intent only: never creates a new autonomous mission.
    // The existing, locally validated mission and Home are required.
    bool requestReturnHome();
    bool requestLowBatteryReturnHome();
    // For subtype 8 true means one bounded best-effort slot was offered,
    // NOT delivered, ACKed or permitted to change actuator state.
    bool sendHardwareTelemetry(const RadioDecodedPayload &status);
    bool sendHardwareTelemetry(const RadioDecodedPayload &status,uint32_t sampledAt);
    // Diagnostic UTC observation only: no link lease, ACK, navigation or
    // actuator authorization. Sent best effort on primary nRF only.
    void offerGnssUtc(const flytec_utc::Sample &utc){utcSource_=utc;}
    bool missionOverrideOutput(int16_t &left,int16_t &right)const;
    bool missionReleaseComplete(uint16_t transaction,uint16_t token,bool success);
    void update();
    void setHomePointStore(const BoatHomePointStore &store){homePointStore_=store;}
    bool failsafeReturnHomeActive()const{return failsafeRthActive_;}
    bool takeCommand(BoatCommand &cmd,bool &duplicateAction,ManagedLink &receivedOn);
    void markActionExecuted(uint8_t sender,uint8_t epoch,uint16_t commandId);
    bool sendFrame(ManagedLink link,const RadioFrame &frame);
    bool sendHeartbeat(ManagedLink link,uint32_t nowMs);
    uint32_t sessionGeneration()const{return generation_;}
    bool completeApplicationAction(const BoatCommand &cmd,uint32_t generation,bool success,uint8_t stopState,uint8_t interlocks,ManagedLink replyLink=LINK_NRF_PRIMARY);
    bool sessionEstablished() const { return sessionStarted_; }
    bool primaryReady() const { return nrfReady_; }
    bool backupReady() const { return sx1262Ready_; }
    bool primaryFresh(uint32_t now) const;
    bool backupFresh(uint32_t now) const;
#if BOAT_R30H_LOCAL_TRACE_ENABLED
    // USB-local source timestamps ONLY; never a cross-board physical STOP measurement.
    bool takeR30HTrace(boat_r30h2::Record &out) { return localTrace_.take(out); }
    uint32_t r30hTraceOverflows() const { return localTrace_.overflows(); }
#endif
    BoatFailsafeDecision evaluateFailsafe(uint32_t now,bool missionActive,bool gnssOK,
                                           bool headingOK,bool batteryOK,bool geofenceOK) const;
private:
#if BOAT_R30H121_ENDPOINT_COMPILE
    flytec_r30h118::Transport<RadioFrame> asyncPrimaryBus_,asyncBackupBus_;
    flytec_r30h118::PhyEvents<RadioFrame> asyncPrimaryEvents_,asyncBackupEvents_;
    flytec_r30h121::DualAppRadio<RadioFrame> asyncRadio_{asyncPrimaryBus_,asyncPrimaryEvents_,asyncBackupBus_,asyncBackupEvents_};
    flytec_r30h120::HandshakeLane<RadioFrame> asyncPrimaryHandshake_,asyncBackupHandshake_;
    flytec_r30h120::SharedPriority asyncPrimaryPriority_,asyncBackupPriority_;
    // Retained for the entire endpoint lifetime. Unsigned compile profile:
    // neither physical radio is commissioned, and the ownership lock is 0.
    flytec_r30h120::PhysicalTask asyncTask_{
      {BOAT_NRF_CE_PIN,BOAT_NRF_CSN_PIN,&SPI,BOAT_NRF_CHANNEL,
       BOAT_NRF_SCK_PIN,BOAT_NRF_MISO_PIN,BOAT_NRF_MOSI_PIN,
       {BOAT_E22_UART_PIN_TX,BOAT_E22_UART_PIN_RX,BOAT_E22_UART_PIN_M0,
        BOAT_E22_UART_PIN_M1,BOAT_E22_UART_PIN_AUX,BOAT_E22_UART_PIN_NRST,
        false,false,false},false,false},
      asyncPrimaryBus_,asyncBackupBus_,asyncPrimaryEvents_,asyncBackupEvents_};
    bool beginAsyncRuntime(bool commissioned);
    void serviceAsyncRuntime(uint32_t now);
    void stopAsyncRuntime();
    flytec_r30h123::OwnerLink<RadioFrame> asyncPrimaryOwner_{asyncRadio_.primary(),asyncPrimaryHandshake_,asyncPrimaryEvents_,asyncPrimaryPriority_};
    flytec_r30h123::OwnerLink<RadioFrame> asyncBackupOwner_{asyncRadio_.backup(),asyncBackupHandshake_,asyncBackupEvents_,asyncBackupPriority_};
    flytec_r30h123::OwnerRx<RadioFrame> asyncPrimaryRx_{asyncRadio_.primary(),0x01,0x02,false};
    flytec_r30h123::OwnerRx<RadioFrame> asyncBackupRx_{asyncRadio_.backup(),0x01,0x02,true};
    flytec_r30h123::OrdinaryTx<RadioFrame> asyncPrimaryTx_,asyncBackupTx_;
    flytec_r30h123::TelemetrySlots asyncTelemetry_;
    void completeAsyncApplication(ManagedLink link,const flytec_r30h118::Ticket &ticket,flytec_r30h121::TxState state,uint32_t now);
    unsigned drainAsyncRx(ManagedLink link,uint32_t now);
    void serviceAsyncOwner(ManagedLink link,uint32_t now,bool linkTrusted);
    bool resetAsyncSession(uint8_t epoch);
    // Queue admission only; caller must correlate terminal completion separately.
    bool queueAsyncHandshake(ManagedLink link,const RadioFrame &frame,uint32_t now);
    bool queueAsyncApplication(ManagedLink link,const RadioFrame &frame,uint32_t now,uint32_t age);
#endif
    flytec_r30h123::RuntimeOwner runtimeOwner_;
    bool legacyBeginAttempted_=false;
    NRF24Link nrf_;
    HardwareSerial e22Serial_{1};
    E22UartBoardPort e22_;
    bool nrfReady_,sx1262Ready_;
    uint8_t key_[32],rootKey_[32];
    SessionHandshake handshake_;
    BoatSessionRecovery recoveryHandshake_;
    BoatHandshakeReply primaryHandshakeReply_,backupHandshakeReply_;
    uint32_t lastSessionRx_=0,generation_=0;
    uint16_t lastReleaseId_=0;
    bool releaseIdSeen_=false;
    uint8_t sessionEpoch_;
    RadioSessionCounter primarySeqGuard_,backupSeqGuard_,crossRadioSeqGuard_;
    BoatCommandDeduplicator dedupe_;
    BoatCommandQueue commandQueue_;
    boat_r30h18::BackupStopAckOutbox backupStopAck_; // independent E22 response slot
    boat_r30h18::StopAckOutbox stopAck_; // STOP-only priority ACK, never a physical STOP witness
#if BOAT_R30H121_ENDPOINT_COMPILE
    flytec_r30h123::StopAckService<RadioFrame,boat_r30h18::StopAckOutbox> asyncPrimaryAck_{asyncPrimaryOwner_,stopAck_,flytec_r30h121::AckLink::Primary};
    flytec_r30h123::StopAckService<RadioFrame,boat_r30h18::BackupStopAckOutbox> asyncBackupAck_{asyncBackupOwner_,backupStopAck_,flytec_r30h121::AckLink::Backup};
#endif
    boat_r30h19::StatusOutbox backupStatusOutbox_;
    boat_r30h19::StatusOutbox statusOutbox_; // Latest real status, never synthesized
#if BOAT_R30H121_ENDPOINT_COMPILE
    flytec_r30h123::StatusService<RadioFrame> asyncPrimaryStatus_{asyncPrimaryOwner_,statusOutbox_};
    flytec_r30h123::StatusService<RadioFrame> asyncBackupStatus_{asyncBackupOwner_,backupStatusOutbox_};
    flytec_r30h123::OwnerCycle<RadioFrame,boat_r30h18::StopAckOutbox> asyncPrimaryCycle_{asyncPrimaryOwner_,asyncPrimaryAck_,asyncPrimaryStatus_,stopAck_,statusOutbox_};
    flytec_r30h123::OwnerCycle<RadioFrame,boat_r30h18::BackupStopAckOutbox> asyncBackupCycle_{asyncBackupOwner_,asyncBackupAck_,asyncBackupStatus_,backupStopAck_,backupStatusOutbox_};
#endif
    MissionAssembler mission_;
    BoatMissionController controller_;
    MissionStickLease stickLease_;
#if BOAT_R30H124W3_NEUTRAL_RECEIPT
    BoatNeutralReceipt neutralReceipt_;
#endif
    BoatRadioWatchdog radioWatchdog_;
#if BOAT_R30H_LOCAL_TRACE_ENABLED
    boat_r30h2::LocalTrace localTrace_{};
    uint32_t lastPrimaryTraceSampleAtMs_=0;
    bool havePrimaryTraceSample_=false;
#endif
    BoatHomePointStore homePointStore_{};
    uint32_t lastAnyAuthenticatedRxMs_=0;
    bool haveAuthenticatedRx_=false;
    bool failsafeRthActive_=false;
    bool linkLossHold_=false;
    bool (*missionInput_)(ExecInputs &)=nullptr;
    ExecOutput missionIntent_={INTENT_STOP,-1,0};
    uint32_t lastMissionStatusMs_=0;
    // One correlated mission reply survives a busy E22 UART; session scoped.
    MissionControlFeedback pendingBackupMissionFeedback_{};
    bool pendingBackupMissionFeedbackValid_=false;
    uint32_t pendingBackupMissionFeedbackAtMs_=0;
    uint32_t lastBackupMissionFeedbackAttemptMs_=0;
    // R30G single best-effort diagnostic slot. STOP/ACK/heartbeat/mission
    // always precede optional telemetry. Never extend command leases.
    boat_r30g::OptionalDiagnosticBudget optionalDiagnostics_{};
    uint32_t lastControlRxMs_=0;
    bool lastControlRxValid_=false;
    bool havePrimaryHeartbeatSent_=false;
    flytec_utc::Sample utcSource_{};
    uint32_t lastUtcTxMs_=0;
    bool utcEverSent_=false;
    bool missionInputs(ExecInputs &in,uint32_t now)const;
    void cancelHandshakeReplies(){
      primaryHandshakeReply_.reset();backupHandshakeReply_.reset();
#if BOAT_R30H121_ENDPOINT_COMPILE
      if(BOAT_R30H121_PHYSICAL_OWNER&&runtimeOwner_.worker()){
        asyncPrimaryOwner_.cancelHandshake();asyncBackupOwner_.cancelHandshake();
      }
#endif
    }
    void noteAuthenticatedRx(uint32_t observedAt){
      const uint32_t now=millis();
      if(!haveAuthenticatedRx_||uint32_t(now-observedAt)<=uint32_t(now-lastAnyAuthenticatedRxMs_))lastAnyAuthenticatedRxMs_=observedAt;
      haveAuthenticatedRx_=true;
      recoveryHandshake_.reset(); // accepted old traffic cancels recovery
      cancelHandshakeReplies();
    }
    bool maybeStartFailsafeReturnHome(uint32_t now);
    bool sendMissionFeedback(const MissionControlFeedback &f,uint32_t now);
    bool primaryOrdinaryTxReady() const;
    // Legacy synchronous admission boundary; not RF delivery or application ACK.
    bool primaryDriverReady() const;
    bool primaryDriverPending() const;
    bool primaryDriverSuccess() const;
    bool backupDriverReady() const;
    bool backupDriverFault() const;
    bool receivePrimaryLegacy(RadioFrame &f);
    bool receiveBackupLegacy(RadioFrame &f);
    bool admitPrimaryLegacy(const RadioFrame &frame);
    bool admitBackupLegacy(const RadioFrame &frame,uint32_t now);
    void prepareApplicationCycle(uint32_t now);
    void serviceLegacyRxAndFeedback();
    void advanceApplicationCycle(uint32_t safetyNow);
    void serviceLegacyPeriodicTx(uint32_t now);
    void abortMission();
    bool (*missionGate_)()=nullptr;
    uint32_t lastNrfRxMs_,lastBackupRxMs_;
    uint32_t lastNrfHeartbeatTxMs_,lastBackupHeartbeatTxMs_,lastBackupHeartbeatAttemptMs_;
    uint16_t txSequence_;
    bool authenticated_;
    bool sessionStarted_;
    bool lastNrfRxValid_,lastBackupRxValid_;
    bool detachedReturn_=false;
    void resetSession(bool preserveMission=false);
    bool sessionReady() const {return sessionStarted_&&txSequence_<RadioSessionCounter::REKEY_AT-1;}
    void receive(ManagedLink link,const RadioFrame &frame);
    void receiveCaptured(ManagedLink link,const RadioFrame &frame,uint32_t capturedAt);
    bool enqueue(const BoatCommand &cmd,ManagedLink link,bool duplicate);
    bool commandPending(uint8_t epoch,uint16_t commandId) const;
    bool sendCommandAck(ManagedLink link,const BoatCommand &cmd,uint8_t result,uint8_t stopState,uint8_t interlockFlags,uint32_t now);
};
