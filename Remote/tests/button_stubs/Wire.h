#pragma once
#include <stdint.h>
class TwoWire {
 uint8_t addr=0,reg=0,count=0;
public:
 uint8_t a=255,b=255,c=255;bool fault=false;
 void beginTransmission(uint8_t x){addr=x;count=0;}
 void write(uint8_t x){if(count++==0)reg=x;}
 uint8_t endTransmission(bool=true){return fault?1:0;}
 uint8_t requestFrom(int,uint8_t){return fault?0:1;}
 int read(){if(reg==0x12)return addr==0x20?a:c;if(reg==0x13)return b;return 255;}
};
extern TwoWire Wire;
