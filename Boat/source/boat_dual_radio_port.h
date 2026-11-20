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
    void update();
    bool takeCommand(BoatCommand &cmd,bool &duplicateAction,ManagedLink &receivedOn);
    void markActionExecuted(uint8_t sender,uint8_t epoch,uint16_t commandId);
    bool sendFrame(ManagedLink link,const RadioFrame &frame);
    bool sendHeartbeat(ManagedLink link,uint32_t nowMs);
    bool sendSessionStart(ManagedLink link,uint32_t nowMs);
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
    uint8_t key_[32];
    uint8_t sessionEpoch_;
    RadioSequenceGuard primarySeqGuard_,backupSeqGuard_,crossRadioSeqGuard_;
    bool sessionStartSeenPrimary_,sessionStartSeenBackup_;
    BoatCommandDeduplicator dedupe_;
    struct QueuedCommand { BoatCommand cmd; ManagedLink link; bool duplicate; uint8_t epoch; };
    QueuedCommand queue_[4];
    uint8_t qWrite_,qRead_,qCount_;
    uint32_t lastNrfRxMs_,lastE22RxMs_;
    uint32_t lastNrfHeartbeatTxMs_,lastE22HeartbeatTxMs_,lastE22HeartbeatAttemptMs_;
    uint16_t txSequence_;
    bool authenticated_;
    bool sessionStarted_;
    bool lastNrfRxValid_,lastE22RxValid_;
    void receive(ManagedLink link,const RadioFrame &frame);
    bool enqueue(const BoatCommand &cmd,ManagedLink link,bool duplicate);
    bool commandPending(uint8_t epoch,uint16_t commandId) const;
    bool sendCommandAck(ManagedLink link,const BoatCommand &cmd,uint8_t result,uint8_t stopState,uint8_t interlockFlags,uint32_t now);
};
