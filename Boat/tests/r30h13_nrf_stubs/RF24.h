#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <SPI.h>
// Deterministic Boat RF24 mock: no ESP peripherals, RF power or GPIO.
constexpr int RF24_250KBPS=250,RF24_PA_LOW=1,RF24_CRC_16=16;
struct FakeRadioBehavior{
    bool beginOk=true,txStarted=true,ack=false,maxRt=false;
    unsigned powerDowns=0,rxFlushes=0,flushes=0,starts=0,listen=0,availableCalls=0,readCalls=0,statusCalls=0;
    uint8_t channel=0,retryDelay=0,retryCount=0,writePipe[5]{},readPipe[5]{};
    int dataRate=-1,paLevel=-1,crcLength=-1,readPipeIndex=-1;
    size_t payloadSize=0;
    bool autoAck=false,dynamicPayloadsDisabled=false;
    uint8_t rxBytes[3][32]{};
    unsigned rxHead=0,rxTail=0,rxCount=0;
    bool inject(const void*src,size_t len){
        if(!src||len!=32U||rxCount>=3U)return false;
        memcpy(rxBytes[rxTail],src,len);
        rxTail=(rxTail+1U)%3U;++rxCount;return true;
    }
};
extern FakeRadioBehavior fakeRadio;
class RF24{
public:
    RF24(int,int,unsigned){}
    bool begin(SPIClass*){return fakeRadio.beginOk;}
    bool isValid()const{return fakeRadio.beginOk;}
    void setChannel(uint8_t ch){fakeRadio.channel=ch;}
    void setDataRate(int rate){fakeRadio.dataRate=rate;}
    void setPALevel(int level){fakeRadio.paLevel=level;}
    void setAutoAck(bool yes){fakeRadio.autoAck=yes;}
    void setRetries(int delay,int count){fakeRadio.retryDelay=(uint8_t)delay;fakeRadio.retryCount=(uint8_t)count;}
    void setCRCLength(int len){fakeRadio.crcLength=len;}
    void disableDynamicPayloads(){fakeRadio.dynamicPayloadsDisabled=true;}
    void setPayloadSize(size_t sz){fakeRadio.payloadSize=sz;}
    void openWritingPipe(const uint8_t*p){memcpy(fakeRadio.writePipe,p,5U);}
    void openReadingPipe(int index,const uint8_t*p){fakeRadio.readPipeIndex=index;memcpy(fakeRadio.readPipe,p,5U);}
    void startListening(){++fakeRadio.listen;}
    void stopListening(){}
    bool startWrite(const void*,size_t,bool){++fakeRadio.starts;return fakeRadio.txStarted;}
    void whatHappened(bool&ok,bool&fail,bool&rxReady){
        ++fakeRadio.statusCalls;
        ok=fakeRadio.ack;fail=fakeRadio.maxRt;rxReady=false;
        fakeRadio.ack=false;fakeRadio.maxRt=false;
    }
    bool available(){++fakeRadio.availableCalls;return fakeRadio.rxCount>0U;}
    void read(void*dst,size_t sz){
        if(fakeRadio.rxCount==0U||sz!=32U)return;
        memcpy(dst,fakeRadio.rxBytes[fakeRadio.rxHead],32U);
        fakeRadio.rxHead=(fakeRadio.rxHead+1U)%3U;
        --fakeRadio.rxCount;++fakeRadio.readCalls;
    }
    void powerDown(){++fakeRadio.powerDowns;}
    void flush_rx(){++fakeRadio.rxFlushes;fakeRadio.rxHead=fakeRadio.rxTail=fakeRadio.rxCount=0;}
    void flush_tx(){++fakeRadio.flushes;}
};
