#pragma once
// R30H-11 remote radio CE on existing MCP23017 @0x21 GPA0.
// The external CE pull-down is necessary for MCP23017 POR high-Z default.
// This is NOT independent RF/motor safety during a stuck MCU or I2C failure.
#include <stdint.h>
class Mcp23017CeBus{
public:
    virtual ~Mcp23017CeBus()=default;
    virtual bool writeReg(uint8_t address,uint8_t reg,uint8_t value)=0;
    virtual bool readReg(uint8_t address,uint8_t reg,uint8_t& value)=0;
};
class Mcp23017CePort{
public:
    explicit Mcp23017CePort(Mcp23017CeBus&bus,uint8_t address=0x21)
        :bus_(bus),address_(address){}
    bool beginLow(); // OLATA low BEFORE output-enable; readback before ready
    bool write(bool high);
    void disable();  // best effort only when I2C is unresponsive
    bool ready()const{return ready_;}
private:
    Mcp23017CeBus&bus_;
    uint8_t address_;
    bool ready_=false;
};