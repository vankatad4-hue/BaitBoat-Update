#include "sc16is750_bridge.h"

static constexpr uint8_t REG_RHR_THR_DLL=0x00;
static constexpr uint8_t REG_IER_DLH=0x01;
static constexpr uint8_t REG_IIR_FCR=0x02;
static constexpr uint8_t REG_LCR=0x03;
static constexpr uint8_t REG_MCR=0x04;
static constexpr uint8_t REG_LSR=0x05;
static constexpr uint8_t REG_TXLVL=0x08;
static constexpr uint8_t REG_RXLVL=0x09;
static constexpr uint8_t REG_IODIR=0x0A;
static constexpr uint8_t REG_IOSTATE=0x0B;
static constexpr uint8_t REG_IOCONTROL=0x0E;
static constexpr uint8_t LSR_DATA_READY=0x01;

SC16IS750Bridge::SC16IS750Bridge(TwoWire &wire,uint8_t address)
    : wire_(wire),address_(address),peekByte_(-1),rxLevelCache_(0),ready_(false) {}

bool SC16IS750Bridge::writeReg(uint8_t reg,uint8_t value) {
    wire_.beginTransmission(address_);
    wire_.write(i2cSubAddress(reg));
    wire_.write(value);
    return wire_.endTransmission()==0;
}
bool SC16IS750Bridge::readReg(uint8_t reg,uint8_t &value) {
    wire_.beginTransmission(address_);
    wire_.write(i2cSubAddress(reg));
    if(wire_.endTransmission(false)!=0) return false;
    if(wire_.requestFrom((int)address_,1)!=1) return false;
    value=(uint8_t)wire_.read();
    return true;
}
bool SC16IS750Bridge::ping() {
    uint8_t lsr=0;
    return readReg(REG_LSR,lsr);
}
bool SC16IS750Bridge::begin(uint32_t baud) {
    ready_=false;
    if(baud!=9600) return false; // initial design crystal/divisor validated for 9600
    // Crystal is 1.8432MHz: divisor = 1.8432MHz/(16*9600)=12.
    if(!writeReg(REG_IER_DLH,0x00)) return false;
    if(!writeReg(REG_LCR,0x80)) return false; // DLAB
    if(!writeReg(REG_RHR_THR_DLL,12)) return false;
    if(!writeReg(REG_IER_DLH,0)) return false;
    if(!writeReg(REG_LCR,0x03)) return false; // 8N1
    if(!writeReg(REG_IIR_FCR,0x07)) return false; // enable/reset FIFOs
    if(!writeReg(REG_MCR,0x00)) return false;
    // GPIO0=M0, GPIO1=M1, GPIO3=/RESET are outputs; GPIO2=AUX input.
    if(!setGpioDirection(0xF4)) return false;
    if(!setGpioState(0x08)) return false; // release E22 reset; normal mode
    delay(2);
    ready_=ping();
    return ready_;
}
bool SC16IS750Bridge::setGpioDirection(uint8_t mask) {
    return writeReg(REG_IODIR,mask);
}
bool SC16IS750Bridge::setGpioState(uint8_t state) {
    return writeReg(REG_IOSTATE,state);
}
bool SC16IS750Bridge::readGpio(uint8_t &state) {
    return readReg(REG_IOSTATE,state);
}
bool SC16IS750Bridge::setE22Mode(bool m0,bool m1) {
    uint8_t state=0;
    if(!readGpio(state)) return false;
    state=(uint8_t)((state & ~0x03) | (m0?0x01:0) | (m1?0x02:0));
    return setGpioState(state);
}
bool SC16IS750Bridge::setE22Reset(bool released) {
    uint8_t state=0;
    if(!readGpio(state)) return false;
    if(released) state|=0x08; else state&=(uint8_t)~0x08;
    return setGpioState(state);
}
bool SC16IS750Bridge::e22AuxReady() {
    uint8_t state=0;
    return readGpio(state) && ((state&0x04)!=0);
}
int SC16IS750Bridge::available() {
    if(!ready_) return 0;
    uint8_t level=0;
    if(!readReg(REG_RXLVL,level)) return (peekByte_>=0 ? 1:0);
    rxLevelCache_=level;
    return level + (peekByte_>=0 ? 1:0);
}
int SC16IS750Bridge::peek() {
    if(peekByte_>=0) return peekByte_;
    if(rxLevelCache_==0){uint8_t level=0;if(!readReg(REG_RXLVL,level))return -1;rxLevelCache_=level;}
    if(rxLevelCache_==0) return -1;
    uint8_t b=0;
    if(readFifo(&b,1)!=1) return -1;
    --rxLevelCache_;
    peekByte_=b;
    return peekByte_;
}
int SC16IS750Bridge::read() {
    if(peekByte_>=0) { int v=peekByte_; peekByte_=-1; return v; }
    if(rxLevelCache_==0){uint8_t level=0;if(!readReg(REG_RXLVL,level))return -1;rxLevelCache_=level;}
    if(rxLevelCache_==0) return -1;
    uint8_t b=0;
    if(readFifo(&b,1)!=1) return -1;
    --rxLevelCache_;
    return b;
}
size_t SC16IS750Bridge::readFifo(uint8_t *data,size_t length) {
    size_t total=0;
    while(total<length) {
        const size_t chunk=(length-total>16)?16:length-total;
        wire_.beginTransmission(address_);
        wire_.write(i2cSubAddress(REG_RHR_THR_DLL));
        if(wire_.endTransmission(false)!=0) break;
        const size_t received=wire_.requestFrom((int)address_,(int)chunk);
        for(size_t i=0;i<received && total<length;i++) data[total++]=(uint8_t)wire_.read();
        if(received!=chunk) break;
    }
    return total;
}
size_t SC16IS750Bridge::writeFifo(const uint8_t *data,size_t length) {
    size_t total=0;
    while(total<length) {
        uint8_t level=0;
        if(!readReg(REG_TXLVL,level)) break;
        size_t freeBytes=64U-level;
        if(freeBytes==0) break;
        size_t chunk=length-total;
        if(chunk>freeBytes) chunk=freeBytes;
        if(chunk>16) chunk=16; // stay within Wire buffer
        wire_.beginTransmission(address_);
        wire_.write(i2cSubAddress(REG_RHR_THR_DLL));
        for(size_t i=0;i<chunk;i++) wire_.write(data[total+i]);
        if(wire_.endTransmission()!=0) break;
        total+=chunk;
    }
    return total;
}
size_t SC16IS750Bridge::write(uint8_t value) { return writeFifo(&value,1); }
size_t SC16IS750Bridge::write(const uint8_t *buffer,size_t size) { return writeFifo(buffer,size); }
void SC16IS750Bridge::flush() {
    const uint32_t start=millis();
    while(millis()-start<100) {
        uint8_t level=0;
        if(!readReg(REG_TXLVL,level) || level==0) break;
        delay(1);
    }
}
