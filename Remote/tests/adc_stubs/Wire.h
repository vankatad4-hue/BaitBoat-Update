#pragma once
#include <stdint.h>
// Deterministic ADS1115 fake used only by the portable regression test.
class TwoWire {
    uint8_t reg_=0,writeCount_=0,high_=0,readCount_=0;
    uint16_t readValue_=0;
    int channel_=0;
public:
    bool error=false;
    int failChannel=-1;
    uint16_t channels[4]={4095,8192,16384,24576};
    void beginTransmission(uint8_t){writeCount_=0;}
    void write(uint8_t v){
        if(writeCount_==0)reg_=v;
        else if(reg_==1&&writeCount_==1)high_=v;
        else if(reg_==1&&writeCount_==2){
            const uint16_t conf=(uint16_t)(((uint16_t)high_<<8)|v);
            channel_=(int)((conf>>12)&0x07U)-4;
        }
        ++writeCount_;
    }
    uint8_t endTransmission(bool=true){return error?1:0;}
    uint8_t requestFrom(int,uint8_t n){
        if(error||n!=2||(reg_==0&&channel_==failChannel))return 0;
        readValue_=reg_==1?0x8000U:channels[channel_];
        readCount_=0;return 2;
    }
    int read(){return readCount_++==0?(int)(readValue_>>8):(int)(readValue_&0xFFU);}
};
extern TwoWire Wire;
