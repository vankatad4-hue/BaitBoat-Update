#pragma once
#include <stdint.h>
#include <stddef.h>
#include <vector>
#include "carrier_safety_core.h"
extern uint32_t fakeNow;
class TwoWire{
public:
 carrier::SafetyCore core;
 carrier::Inputs inputs{};
 uint16_t sample=0;
 bool failWrite=false,failRead=false,corrupt=false,freeze=false;
 std::vector<uint8_t> sent,received;size_t at=0;
 void beginTransmission(uint8_t address){sent.clear();if(address!=carrier::ADDRESS)failWrite=true;}
 size_t write(const uint8_t *p,size_t n){sent.assign(p,p+n);return n;}
 uint8_t endTransmission(){carrier::Command c{};if(failWrite)return 4;if(!carrier::decode(sent.data(),sent.size(),c))return 4;core.receive(c,fakeNow,inputs);core.tick(fakeNow,inputs);return 0;}
 size_t requestFrom(uint8_t address,uint8_t n){if(failRead||address!=carrier::ADDRESS)return 0;uint8_t b[carrier::SIZE];if(!freeze)++sample;carrier::encodeStatus(core.status(sample,inputs),b);if(corrupt)b[4]^=1;received.assign(b,b+n);at=0;return n;}
 int read(){return at<received.size()?received[at++]:-1;}
};
