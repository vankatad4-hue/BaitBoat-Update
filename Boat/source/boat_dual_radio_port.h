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
#include "boat_command_queue.h"
#include "mission_wire.h"
#include "mission_controller.h"
#include "mission_stick.h"
#include "radio_session_platform.h"

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
    NRF24Link nrf_;
    HardwareSerial e22Serial_{1};
    E22UartBoardPort e22_;
    bool nrfReady_,sx1262Ready_;
    uint8_t key_[32],rootKey_[32];
    SessionHandshake handshake_;
    uint32_t lastSessionRx_=0,generation_=0;
    uint16_t lastReleaseId_=0;
    bool releaseIdSeen_=false;
    uint8_t sessionEpoch_;
    RadioSessionCounter primarySeqGuard_,backupSeqGuard_,crossRadioSeqGuard_;
    BoatCommandDeduplicator dedupe_;
    BoatCommandQueue commandQueue_;
    MissionAssembler mission_;
    BoatMissionController controller_;
    MissionStickLease stickLease_;
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
    void noteAuthenticatedRx(uint32_t now){lastAnyAuthenticatedRxMs_=now;haveAuthenticatedRx_=true;}
    bool maybeStartFailsafeReturnHome(uint32_t now);
    bool sendMissionFeedback(const MissionControlFeedback &f,uint32_t now);
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
    bool enqueue(const BoatCommand &cmd,ManagedLink link,bool duplicate);
    bool commandPending(uint8_t epoch,uint16_t commandId) const;
    bool sendCommandAck(ManagedLink link,const BoatCommand &cmd,uint8_t result,uint8_t stopState,uint8_t interlockFlags,uint32_t now);
};
