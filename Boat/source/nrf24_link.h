#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include "radio_protocol.h"

class NRF24Link {
public:
    NRF24Link(int cePin,int csnPin,SPIClass *spiBus,uint8_t channel=76,int sck=-1,int miso=-1,int mosi=-1);
    bool begin(bool remoteRole);
    void update();
    bool sendFrame(const RadioFrame &frame);
    bool receiveFrame(RadioFrame &frame);
    bool ready() const { return ready_; }
    bool txPending() const { return txPending_; }
    bool lastTxSuccess() const { return lastTxSuccess_; }
    uint32_t txCount() const { return txCount_; }
    uint32_t rxCount() const { return rxCount_; }
    uint32_t txFailCount() const { return txFailCount_; }
private:
    RF24 radio_;
    SPIClass *spiBus_;
    int cePin_,csnPin_;
    uint8_t channel_;
    int spiSck_,spiMiso_,spiMosi_;
    bool ready_,txPending_,lastTxSuccess_;
    uint32_t txCount_,rxCount_,txFailCount_;
    // Software-only transport lock. This is NOT a physical STOP watchdog.
    static constexpr uint32_t TX_PENDING_TIMEOUT_MS = 120U;
    uint32_t txStartedAtMs_ = 0;
    RadioFrame rxQueue_[4];
    uint8_t rxHead_,rxTail_,rxQueueCount_;
    const uint8_t *txAddress_;
    const uint8_t *rxAddress_;
};
