#include "sc16is750_bridge.h"
#include "remote_i2c_owner.h"
#include "sc16is750_fifo_budget.h"

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
    if(!flytec_rtos::remoteI2cAllowed())return false;
    wire_.beginTransmission(address_);
    wire_.write(i2cSubAddress(reg));
    wire_.write(value);
    return wire_.endTransmission()==0;
}
bool SC16IS750Bridge::readReg(uint8_t reg,uint8_t &value) {
    if(!flytec_rtos::remoteI2cAllowed())return false;
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
    if(commissioningLocked_)return false;
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
    if(!setGpioDirection(r30h_sc16::E22_GPIO_OUTPUT_MASK)) return false;
    uint8_t direction=0,state=0;
    if(!readReg(REG_IODIR,direction)||direction!=r30h_sc16::E22_GPIO_OUTPUT_MASK) return false;
    if(!setGpioState(0x08)) return false; // release E22 reset; normal mode
    if(!readGpio(state)||(state&0x0BU)!=0x08U)return false;
    delay(2);
    ready_=ping();
    return ready_;
}
bool SC16IS750Bridge::commissioningAllowed()const {
    return commissioningLocked_&&!commissioningFault_&&flytec_rtos::remoteI2cAllowed()&&
           wire_.getTimeOut()>0&&wire_.getTimeOut()<=20U&&isolation_&&isolation_(isolationContext_);
}
bool SC16IS750Bridge::lockForCommissioning(const flytec_rtos::E22CommissioningPermit& permit,
                                           flytec_rtos::E22IsolationProbe isolation,void* context) {
    if(commissioningLocked_||!ready_||!permit.valid()||!flytec_rtos::remoteI2cAllowed()||
       !wire_.getTimeOut()||wire_.getTimeOut()>20U||!isolation||!isolation(context))return false;
    commissioningLocked_=true;ready_=false;isolation_=isolation;isolationContext_=context;
    auto read=[&](uint8_t r,uint8_t& v){return commissioningAllowed()&&readReg(r,v);};
    uint8_t direction=0;
    if(peekByte_>=0||!read(REG_IODIR,direction)||direction!=r30h_sc16::E22_GPIO_OUTPUT_MASK||
       !flytec_rtos::sc16CommissioningDrained(read)||!e22AuxReady()||!commissioningAllowed()){
        commissioningFault_=true;return false;
    }
    rxLevelCache_=0;return true;
}
bool SC16IS750Bridge::commissioningBaudVerified(uint32_t crystalHz,uint32_t baud,uint32_t limitMs) {
    if(!commissioningAllowed()||!limitMs||limitMs>500U||!flytec_rtos::sc16CommissioningDivisor(crystalHz,baud))return false;
    const uint32_t started=millis();
    auto read=[&](uint8_t r,uint8_t& v){return commissioningAllowed()&&uint32_t(millis()-started)<limitMs&&readReg(r,v);};
    auto write=[&](uint8_t r,uint8_t v){return commissioningAllowed()&&uint32_t(millis()-started)<limitMs&&writeReg(r,v);};
    const bool ok=e22AuxReady()&&flytec_rtos::sc16CommissioningSetBaud(read,write,crystalHz,baud)&&
                  uint32_t(millis()-started)<limitMs&&commissioningAllowed();
    if(!ok)commissioningFault_=true;
    return ok;
}
bool SC16IS750Bridge::commissioningModeMatches(bool config){
    uint8_t dir=0,state=0;
    return commissioningAllowed()&&readReg(REG_IODIR,dir)&&dir==r30h_sc16::E22_GPIO_OUTPUT_MASK&&
           readGpio(state)&&(state&0x0BU)==(config?0x0AU:0x08U);
}
bool SC16IS750Bridge::commissioningModeVerified(bool config,uint32_t limit){
    const uint32_t start=millis();auto read=[&](uint8_t r,uint8_t& v){return commissioningAllowed()&&readReg(r,v);};
    if(!commissioningAllowed()||!limit||limit>500U||!flytec_rtos::sc16CommissioningDrained(read)||!e22AuxReady()||!setE22Mode(false,config)){
        commissioningFault_=true;return false;
    }
    delay(3);
    while(commissioningAllowed()&&uint32_t(millis()-start)<limit){
        if(commissioningModeMatches(config)&&e22AuxReady()){
            delay(3);if(uint32_t(millis()-start)<limit&&commissioningModeMatches(config)&&e22AuxReady())return true;
            break;
        }
        delay(1);
    }
    commissioningFault_=true;return false;
}
bool SC16IS750Bridge::commissioningRxCount(unsigned& count){
    uint8_t lcr=0,lsr=0,rx=0;
    if(!commissioningAllowed()||!readReg(REG_LCR,lcr)||lcr!=3||!readReg(REG_LSR,lsr)||(lsr&0x9EU)||
       !readReg(REG_RXLVL,rx)||rx>64)return false;
    count=rx;return true;
}
bool SC16IS750Bridge::commissioningReadByte(uint8_t& b){
    unsigned count=0;return commissioningRxCount(count)&&count&&readFifo(&b,1)==1;
}
bool SC16IS750Bridge::commissioningTxIdle(){
    uint8_t lcr=0,lsr=0,tx=0;
    return commissioningAllowed()&&readReg(REG_LCR,lcr)&&lcr==3&&readReg(REG_TXLVL,tx)&&tx==64&&
           readReg(REG_LSR,lsr)&&(lsr&0x40U)&&!(lsr&0x9EU)&&e22AuxReady();
}
bool SC16IS750Bridge::commissioningConfigWrite(const uint8_t* cmd,unsigned n){
    if(!commissioningModeMatches(true)||!commissioningTxIdle()||!flytec_rtos::e22CommissioningCommand(cmd,n,n==3?12:4))return false;
    // One small I2C FIFO transaction, only with DLAB verified clear.
    wire_.beginTransmission(address_);wire_.write(i2cSubAddress(REG_RHR_THR_DLL));
    for(unsigned i=0;i<n;++i)wire_.write(cmd[i]);
    return wire_.endTransmission()==0;
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
int SC16IS750Bridge::availableForWrite() {
    uint8_t level=0;
    return ready_&&readReg(REG_TXLVL,level)?r30h_sc16::freeTxSlots(level):0;
}
int SC16IS750Bridge::available() {
    if(!ready_) return 0;
    uint8_t level=0;
    if(!readReg(REG_RXLVL,level)) return (peekByte_>=0 ? 1:0);
    rxLevelCache_=level;
    return level + (peekByte_>=0 ? 1:0);
}
int SC16IS750Bridge::peek() {
    if(commissioningLocked_)return -1;
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
    if(commissioningLocked_)return -1;
    if(peekByte_>=0) { int v=peekByte_; peekByte_=-1; return v; }
    if(rxLevelCache_==0){uint8_t level=0;if(!readReg(REG_RXLVL,level))return -1;rxLevelCache_=level;}
    if(rxLevelCache_==0) return -1;
    uint8_t b=0;
    if(readFifo(&b,1)!=1) return -1;
    --rxLevelCache_;
    return b;
}
size_t SC16IS750Bridge::readFifo(uint8_t *data,size_t length) {
    if(!flytec_rtos::remoteI2cAllowed())return 0;
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
    if(commissioningLocked_)return 0;
    if(!flytec_rtos::remoteI2cAllowed())return 0;
    size_t total=0;
    while(total<length) {
        uint8_t level=0;
        if(!readReg(REG_TXLVL,level)) break;
        // TXLVL reports the number of FREE byte positions (0x40=empty).
        // The previous 64-level inversion prevented writes to an empty FIFO.
        const size_t freeBytes=r30h_sc16::freeTxSlots(level);
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
    if(commissioningLocked_)return;
    const uint32_t start=millis();
    while(millis()-start<100) {
        uint8_t level=0;
        // Drain only when every slot is free, not when TX FIFO is FULL.
        if(!readReg(REG_TXLVL,level) || r30h_sc16::txFifoEmpty(level)) break;
        delay(1);
    }
}

// Compile the offline binding against the real bridge; never invoke it here.
template class flytec_rtos::E22OfflinePort<SC16IS750Bridge>;
