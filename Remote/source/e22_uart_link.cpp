#include "e22_uart_link.h"
#include <string.h>

static const uint8_t E22_WRITE_CMD[12]={0xC0,0x00,0x09,0x00,0x00,0x00,0x67,0xC3,0x12,0x00,0x00,0x00};
static const uint8_t E22_WRITE_ACK[12]={0xC1,0x00,0x09,0x00,0x00,0x00,0x67,0xC3,0x12,0x00,0x00,0x00};
static const uint8_t E22_READ_CMD[3]={0xC1,0x00,0x09};

E22UartLink::E22UartLink(Stream &uart,E22SetModeFn mode,E22AuxReadyFn aux,E22ResetFn reset,void *ctx,E22TxCapacityFn capacityFn)
 : uart_(uart),modeFn_(mode),auxFn_(aux),resetFn_(reset),context_(ctx),capacityFn_(capacityFn),configured_(false),
   auxReadyNow_(false),rxIndex_(0),rxHead_(0),rxTail_(0),rxQueueCount_(0),
   txCount_(0),rxCount_(0),rejectedDutyCount_(0) {
    memset(rxBuffer_,0,sizeof(rxBuffer_)); memset(rxQueue_,0,sizeof(rxQueue_)); clearBuckets();
}
void E22UartLink::clearBuckets(){for(auto &b:buckets_){b.minuteStamp=0;b.airtimeMs=0;b.valid=false;}}
uint32_t E22UartLink::usedAirtimeMs(uint32_t now){
    const uint32_t currentMinute=now/60000U; uint32_t total=0;
    for(auto &b:buckets_) if(b.valid && (currentMinute-b.minuteStamp)<60U) total+=b.airtimeMs;
    return total;
}
bool E22UartLink::dutyAllows(uint32_t now){return usedAirtimeMs(now)+EST_AIRTIME_MS<=MAX_AIRTIME_PER_HOUR_MS;}
void E22UartLink::recordAirtime(uint32_t now,uint32_t ms){
    const uint32_t minute=now/60000U; auto &b=buckets_[minute%60U];
    if(!b.valid || b.minuteStamp!=minute){b.minuteStamp=minute;b.airtimeMs=0;b.valid=true;}
    uint32_t sum=(uint32_t)b.airtimeMs+ms; b.airtimeMs=(sum>65535U)?65535U:(uint16_t)sum;
}
bool E22UartLink::configCommand(const uint8_t *cmd,size_t length,const uint8_t *expected,size_t expectedLen,uint32_t timeout){
    while(uart_.available()) uart_.read();
    if(uart_.write(cmd,length)!=length) return false;
    uart_.flush();
    uint8_t response[16]; size_t n=0; const uint32_t start=millis();
    while((uint32_t)(millis()-start)<timeout && n<expectedLen){
        while(uart_.available() && n<expectedLen) response[n++]=(uint8_t)uart_.read();
        if(n==expectedLen) break;
        delay(1);
    }
    if(n!=expectedLen) return false;
    return memcmp(response,expected,expectedLen)==0;
}
bool E22UartLink::configureEU868TestProfile(){
    configured_=false;
    txState_.reset();rxIndex_=rxHead_=rxTail_=rxQueueCount_=0;
    if(!modeFn_ || !auxFn_) return false;
    if(resetFn_){resetFn_(false,context_);delay(5);resetFn_(true,context_);delay(20);}
    modeFn_(false,true,context_); // M1=1, M0=0 configuration mode
    const uint32_t waitStart=millis();
    while(!auxFn_(context_) && (uint32_t)(millis()-waitStart)<500) delay(1);
    if(!auxFn_(context_)) { modeFn_(false,false,context_); return false; }
    const bool writeOK=configCommand(E22_WRITE_CMD,sizeof(E22_WRITE_CMD),E22_WRITE_ACK,sizeof(E22_WRITE_ACK),250);
    const bool readOK=writeOK && configCommand(E22_READ_CMD,sizeof(E22_READ_CMD),E22_WRITE_ACK,sizeof(E22_WRITE_ACK),250);
    modeFn_(false,false,context_); // transparent normal mode even on failed config
    const uint32_t normalStart=millis();
    while(!auxFn_(context_) && (uint32_t)(millis()-normalStart)<500) delay(1);
    configured_=writeOK && readOK && auxFn_(context_);
    auxReadyNow_=configured_;
    return configured_;
}
void E22UartLink::update(){
    auxReadyNow_=auxFn_?auxFn_(context_):false;
    txState_.poll(millis(),auxReadyNow_);
    if(txState_.fault()){configured_=false;rxIndex_=rxHead_=rxTail_=rxQueueCount_=0;return;}
    if(!configured_)return;
    uint8_t processed=0;
    while(processed<32U && rxQueueCount_<4U && uart_.available()){
        const int value=uart_.read();if(value<0)break;
        parserByte((uint8_t)value);++processed;
    }
}
void E22UartLink::parserByte(uint8_t v){
    if(rxIndex_==0){if(v==RADIO_FRAME_MAGIC0) rxBuffer_[rxIndex_++]=v;return;}
    if(rxIndex_==1){if(v==RADIO_FRAME_MAGIC1) rxBuffer_[rxIndex_++]=v;else {rxIndex_=(v==RADIO_FRAME_MAGIC0)?1:0;}return;}
    rxBuffer_[rxIndex_++]=v;
    if(rxIndex_>=RADIO_FRAME_SIZE){
        RadioFrame f; memcpy(&f,rxBuffer_,sizeof(f)); rxIndex_=0;
        if(f.magic[0]==RADIO_FRAME_MAGIC0 && f.magic[1]==RADIO_FRAME_MAGIC1 &&
           radio_crc16((const uint8_t*)&f,offsetof(RadioFrame,crc16))==f.crc16){
            if(rxQueueCount_<4){rxQueue_[rxTail_]=f;rxTail_=(uint8_t)((rxTail_+1U)%4U);++rxQueueCount_;++rxCount_;}
        }
    }
}
bool E22UartLink::sendFrame(const RadioFrame &frame,uint32_t now){
    update();
    return trySendFrame(frame,now)==Admission::Accepted;
}
E22UartLink::Admission E22UartLink::trySendFrame(const RadioFrame &frame,uint32_t now){
    if(txState_.fault())return Admission::Uncertain;
    if(!configured_)return Admission::Unavailable;
    if(!auxReadyNow_ || txState_.pending())return Admission::Busy;
    if(!dutyAllows(now)){++rejectedDutyCount_;return Admission::DutyLimited;}
    if(frame.magic[0]!=RADIO_FRAME_MAGIC0 || frame.magic[1]!=RADIO_FRAME_MAGIC1)return Admission::Invalid;
    if(capacityFn_ && capacityFn_(context_)<sizeof(frame))return Admission::Busy;
    const size_t n=uart_.write((const uint8_t*)&frame,sizeof(frame));
    // Even a reported zero write can be ambiguous on a failed bridge transaction.
    recordAirtime(now,EST_AIRTIME_MS);
    if(n!=sizeof(frame)){
        txState_.uncertain();configured_=false;rxIndex_=rxHead_=rxTail_=rxQueueCount_=0;
        return Admission::Uncertain;
    }
    ++txCount_;txState_.accepted(millis());
    return Admission::Accepted;
}
bool E22UartLink::receiveFrame(RadioFrame &frame){
    if(!configured_||rxQueueCount_==0) return false;
    frame=rxQueue_[rxHead_];rxHead_=(uint8_t)((rxHead_+1U)%4U);--rxQueueCount_;return true;
}
