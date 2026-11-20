#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <HardwareSerial.h>
#include "nrf24_link.h"
#include "e22_uart_link.h"
#include "radio_command_codec.h"
#include "radio_key_store.h"
#include "boat_radio_safety.h"
#include "radio_payload_encoder.h"
#include "boat_command_queue.h"
#include "mission_wire.h"
#include "mission_controller.h"
#include "mission_stick.h"
#include "radio_session_platform.h"

// Link values match the Remote manager and queue records.
enum ManagedLink : uint8_t { LINK_NONE=0, LINK_NRF_PRIMARY=1, LINK_LORA_BACKUP=2 };

// All physical GPIO values must be audited/frozen for the exact Tracker SKU.
// Default -1 macros intentionally keep radio hardware disabled.
#ifndef BOAT_NRF_CE_PIN
#define BOAT_NRF_CE_PIN (-1)
#endif
#ifndef BOAT_NRF_CSN_PIN
#define BOAT_NRF_CSN_PIN (-1)
#endif
#ifndef BOAT_E22_UART_RX_PIN
#define BOAT_E22_UART_RX_PIN (-1)
#endif
#ifndef BOAT_E22_UART_TX_PIN
#define BOAT_E22_UART_TX_PIN (-1)
#endif
#ifndef BOAT_E22_M0_PIN
#define BOAT_E22_M0_PIN (-1)
#endif
#ifndef BOAT_E22_M1_PIN
#define BOAT_E22_M1_PIN (-1)
#endif
#ifndef BOAT_E22_AUX_PIN
#define BOAT_E22_AUX_PIN (-1)
#endif
#ifndef BOAT_E22_RESET_PIN
#define BOAT_E22_RESET_PIN (-1)
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
    void localEmergencyStop(){const bool wasDetached=detachedReturn_;abortMission();detachedReturn_=false;if(wasDetached)controller_.resetSession();commandQueue_.clear();mission_.reset();}
    bool confirmManual(){ExecInputs in{};return missionInputs(in,millis())&&controller_.confirmManual(in);}
    bool sendHardwareTelemetry(const RadioDecodedPayload &status);
    bool missionOverrideOutput(int16_t &left,int16_t &right)const;
    bool missionReleaseComplete(uint16_t transaction,uint16_t token,bool success);
    void update();
    bool takeCommand(BoatCommand &cmd,bool &duplicateAction,ManagedLink &receivedOn);
    void markActionExecuted(uint8_t sender,uint8_t epoch,uint16_t commandId);
    bool sendFrame(ManagedLink link,const RadioFrame &frame);
    bool sendHeartbeat(ManagedLink link,uint32_t nowMs);
    uint32_t sessionGeneration()const{return generation_;}
    bool completeApplicationAction(const BoatCommand &cmd,uint32_t generation,bool success,uint8_t stopState,uint8_t interlocks);
    bool sessionEstablished() const { return sessionStarted_; }
    bool primaryReady() const { return nrfReady_; }
    bool backupReady() const { return e22Configured_; }
    bool primaryFresh(uint32_t now) const;
    bool backupFresh(uint32_t now) const;
    BoatFailsafeDecision evaluateFailsafe(uint32_t now,bool missionActive,bool gnssOK,
                                           bool headingOK,bool batteryOK,bool geofenceOK) const;
private:
    HardwareSerial serial_;
    NRF24Link nrf_;
    E22UartLink e22_;
    bool nrfReady_,e22Configured_;
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
    bool (*missionInput_)(ExecInputs &)=nullptr;
    ExecOutput missionIntent_={INTENT_STOP,-1,0};
    uint32_t lastMissionStatusMs_=0;
    bool missionInputs(ExecInputs &in,uint32_t now)const;
    bool sendMissionFeedback(const MissionControlFeedback &f,uint32_t now);
    void abortMission();
    bool (*missionGate_)()=nullptr;
    uint32_t lastNrfRxMs_,lastE22RxMs_;
    uint32_t lastNrfHeartbeatTxMs_,lastE22HeartbeatTxMs_,lastE22HeartbeatAttemptMs_;
    uint16_t txSequence_;
    bool authenticated_;
    bool sessionStarted_;
    bool lastNrfRxValid_,lastE22RxValid_;
    bool detachedReturn_=false;
    void resetSession(bool preserveReturn=false);
    bool sessionReady() const {return sessionStarted_&&txSequence_<RadioSessionCounter::REKEY_AT-1;}
    void receive(ManagedLink link,const RadioFrame &frame);
    bool enqueue(const BoatCommand &cmd,ManagedLink link,bool duplicate);
    bool commandPending(uint8_t epoch,uint16_t commandId) const;
    bool sendCommandAck(ManagedLink link,const BoatCommand &cmd,uint8_t result,uint8_t stopState,uint8_t interlockFlags,uint32_t now);
};
