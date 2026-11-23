#include "mcp23017_ce_port.h"
namespace {
constexpr uint8_t IOCON=0x0a,IODIRA=0x00,GPPUA=0x0c,GPIOA=0x12,OLATA=0x14,CE=0x01;
}
bool Mcp23017CePort::beginLow(){
    ready_=false;
    // The other I2C expander, 0x20, owns critical physical STOP/release keys.
    // GPA4 at 0x21 is the physical OK key and remains an INPUT.
    if(address_!=0x21)return false;
    uint8_t iocon=0,olat=0,dir=0,pull=0,actual=0;
    if(!bus_.readReg(address_,IOCON,iocon)||(iocon&0x80U)||
       !bus_.readReg(address_,OLATA,olat)||
       !bus_.writeReg(address_,OLATA,(uint8_t)(olat&~CE))||
       !bus_.readReg(address_,GPPUA,pull)||
       !bus_.writeReg(address_,GPPUA,(uint8_t)(pull&~CE))||
       !bus_.readReg(address_,IODIRA,dir)||
       !bus_.writeReg(address_,IODIRA,(uint8_t)(dir&~CE))||
       !bus_.readReg(address_,IODIRA,actual)||(actual&CE)!=0U||
       !bus_.readReg(address_,GPIOA,actual)||(actual&CE)!=0U){
        disable();return false;
    }
    ready_=true;return true;
}
bool Mcp23017CePort::write(bool high){
    if(!ready_)return false;
    uint8_t latch=0,actual=0;
    if(!bus_.readReg(address_,OLATA,latch)||
       !bus_.writeReg(address_,OLATA,high?(uint8_t)(latch|CE):
                                          (uint8_t)(latch&~CE))||
       !bus_.readReg(address_,GPIOA,actual)||
       (((actual&CE)!=0U)!=high)){
        disable();return false;
    }
    return true;
}
void Mcp23017CePort::disable(){
    ready_=false;
    // All other GPA0-7 pins on the existing 0x21 board are inputs by policy.
    // Overwriting the latch to 0 is safe for this current design, but it
    // cannot stop a radio when the I2C bus is physically broken.
    (void)bus_.writeReg(address_,OLATA,0);
}