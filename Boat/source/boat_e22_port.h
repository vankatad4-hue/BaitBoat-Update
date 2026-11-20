#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>
#include "e22_uart_link.h"
#include "radio_command_codec.h"
#include "boat_radio_safety.h"
#include "radio_key_store.h"

// Direct UART adapter for the boat. Pin values remain unassigned until the
// exact Heltec Wireless Tracker revision pin audit is complete.
#ifndef BOAT_E22_UART_RX_PIN
#define BOAT_E22_UART_RX_PIN -1
#endif
#ifndef BOAT_E22_UART_TX_PIN
#define BOAT_E22_UART_TX_PIN -1
#endif
#ifndef BOAT_E22_M0_PIN
#define BOAT_E22_M0_PIN -1
#endif
#ifndef BOAT_E22_M1_PIN
#define BOAT_E22_M1_PIN -1
#endif
#ifndef BOAT_E22_AUX_PIN
#define BOAT_E22_AUX_PIN -1
#endif
#ifndef BOAT_E22_RESET_PIN
#define BOAT_E22_RESET_PIN -1
#endif

class BoatE22Port {
public:
    BoatE22Port();
    bool begin();
    void update();
    bool takeCommand(BoatCommand &command,bool &duplicateAction);
    void markActionExecuted(uint8_t source,uint8_t epoch,uint16_t commandId);
    bool ready() const { return ready_; }
private:
    static void modePins(bool m0,bool m1,void *ctx);
    static bool auxReady(void *ctx);
    static void resetPin(bool released,void *ctx);
    HardwareSerial serial_;
    E22UartLink link_;
    bool ready_;
    uint8_t key_[32];
    RadioSequenceGuard sequenceGuard_;
    BoatCommandDeduplicator dedupe_;
    RadioFrame rxFrame_;
    bool rxAvailable_;
    bool duplicateAction_;
    BoatCommand decoded_;
};
