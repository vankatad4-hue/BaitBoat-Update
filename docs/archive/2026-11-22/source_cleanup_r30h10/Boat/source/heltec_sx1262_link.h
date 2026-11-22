#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include "boat_radio_hw_config.h"
#include "radio_protocol.h"
#include "heltec_sx1262_airtime.h"
// Native Heltec SX1262, disabled by default; no E22 transparent UART.
class HeltecSx1262Link{
public:
 HeltecSx1262Link();
 bool begin();
 void update();
 bool sendFrame(const RadioFrame &frame,uint32_t now);
 bool receiveFrame(RadioFrame &frame);
 bool ready()const{return ready_;}
private:
 Module module_;
 SX1262 radio_;
 Sx1262DutyLimiter limiter_;
 bool ready_=false,txPending_=false;
 uint32_t started_=0;
 RadioFrame queue_[4]{};
 uint8_t head_=0,tail_=0,count_=0;
};
