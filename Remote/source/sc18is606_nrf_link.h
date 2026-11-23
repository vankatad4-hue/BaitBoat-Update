#pragma once
#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>
#include "sc18is606_port.h"
#include "radio_protocol.h"
// Dedicated I2C->SPI bridge nRF24 transport for Waveshare Standard.
// No physical direct CE/CSN/IRQ GPIO; separate from TF-card SPI.
// RF is gated OFF in boat_radio.cpp until independent physical signoff.
class Sc18is606NrfLink {
public:
    Sc18is606NrfLink(Sc18is606Io&io,Mcp23017CePort&ce,uint8_t address,uint8_t channel);
    bool begin(bool remoteRole);
    void update();
    bool sendFrame(const RadioFrame&frame);
    bool receiveFrame(RadioFrame&frame);
    bool ready()const{return ready_&&port_.ready();}
    bool txPending()const{return txPending_;}
    bool lastTxSuccess()const{return lastTxSuccess_;}
    uint32_t txCount()const{return txCount_;}
    uint32_t rxCount()const{return rxCount_;}
    uint32_t txFailCount()const{return txFailCount_;}
private:
    bool regWrite(uint8_t reg,uint8_t value);
    bool regRead(uint8_t reg,uint8_t&value);
    bool regWriteBytes(uint8_t reg,const uint8_t*data,size_t count);
    bool command(uint8_t code);
    bool pollStatus(uint8_t& status);
    bool listen();
    bool queueRx();
    void trip();
    Sc18is606Port port_;
    uint8_t channel_;
    bool ready_=false,txPending_=false,lastTxSuccess_=false;
    uint32_t txAt_=0,txCount_=0,rxCount_=0,txFailCount_=0;
    RadioFrame rxQueue_[4]{};
    uint8_t rxHead_=0,rxTail_=0,rxCountQueued_=0;
    const uint8_t*txAddress_=nullptr;
    const uint8_t*rxAddress_=nullptr;
};
