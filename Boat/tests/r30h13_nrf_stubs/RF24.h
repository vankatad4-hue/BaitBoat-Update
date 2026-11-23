#pragma once
#include <stddef.h>
#include <stdint.h>
#include <SPI.h>
// Logic-only fake for the Boat RF24 transport watchdog; NEVER drives GPIO.
constexpr int RF24_250KBPS=0,RF24_PA_LOW=0,RF24_CRC_16=0;
struct FakeRadioBehavior{
    bool beginOk=true,txStarted=true,ack=false,maxRt=false;
    unsigned flushes=0,starts=0,listen=0;
};
extern FakeRadioBehavior fakeRadio;
class RF24{
public:
    RF24(int,int,unsigned){}
    bool begin(SPIClass*){return fakeRadio.beginOk;}
    bool isValid()const{return fakeRadio.beginOk;}
    void setChannel(uint8_t){}
    void setDataRate(int){}
    void setPALevel(int){}
    void setAutoAck(bool){}
    void setRetries(int,int){}
    void setCRCLength(int){}
    void disableDynamicPayloads(){}
    void setPayloadSize(size_t){}
    void openWritingPipe(const uint8_t*){}
    void openReadingPipe(int,const uint8_t*){}
    void startListening(){++fakeRadio.listen;}
    void stopListening(){}
    bool startWrite(const void*,size_t,bool){++fakeRadio.starts;return fakeRadio.txStarted;}
    void whatHappened(bool&ok,bool&fail,bool&rxReady){
        ok=fakeRadio.ack;fail=fakeRadio.maxRt;rxReady=false;
        fakeRadio.ack=false;fakeRadio.maxRt=false;
    }
    bool available(){return false;}
    void read(void*,size_t){}
    void flush_tx(){++fakeRadio.flushes;}
};
