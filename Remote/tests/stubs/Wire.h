#pragma once
#include <stdint.h>
class TwoWire{public:void beginTransmission(uint8_t){};void write(uint8_t){};uint8_t endTransmission(bool stop=true){(void)stop;return 0;}uint8_t requestFrom(int,uint8_t){return 1;}int read(){return 0xFF;}};extern TwoWire Wire;
