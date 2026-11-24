#include "mcp23017_ce_wire_io.h"
#include "remote_i2c_owner.h"
bool Mcp23017CeWireIo::writeReg(uint8_t addr,uint8_t reg,uint8_t value){
    if(!flytec_rtos::remoteI2cAllowed())return false;
    wire_.beginTransmission(addr);
    if(wire_.write(reg)!=1||wire_.write(value)!=1)return false;
    return wire_.endTransmission()==0;
}
bool Mcp23017CeWireIo::readReg(uint8_t addr,uint8_t reg,uint8_t&value){
    if(!flytec_rtos::remoteI2cAllowed())return false;
    wire_.beginTransmission(addr);
    if(wire_.write(reg)!=1||wire_.endTransmission(false)!=0)return false;
    if(wire_.requestFrom((int)addr,1)!=1)return false;
    const int raw=wire_.read();if(raw<0)return false;
    value=(uint8_t)raw;return true;
}