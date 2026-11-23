#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "mcp23017_ce_port.h"
class Mcp23017CeWireIo final:public Mcp23017CeBus{
public:
    explicit Mcp23017CeWireIo(TwoWire& wire):wire_(wire){}
    bool writeReg(uint8_t addr,uint8_t reg,uint8_t value) override;
    bool readReg(uint8_t addr,uint8_t reg,uint8_t&value) override;
private:
    TwoWire&wire_;
};