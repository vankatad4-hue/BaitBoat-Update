#include "sc18is606_port.h"
#include <string.h>

bool Sc18is606Port::command(const uint8_t*data,size_t count,bool retry){
    if(!data||count==0||count>MAX_TRANSFER+1)return false;
    // NXP bridge temporarily NACKs when busy. Non-idempotent SPI commands
    // NEVER retry: an ambiguous I2C failure could duplicate a transmitted frame.
    const unsigned attempts=retry?5U:1U;
    for(unsigned i=0;i<attempts;i++){
        if(io_.write(address_,data,count))return true;
        if(i+1U<attempts)io_.pauseMs(1);
    }
    return false;
}
bool Sc18is606Port::fetch(uint8_t*data,size_t count){
    if(!data||count==0||count>MAX_TRANSFER)return false;
    for(unsigned i=0;i<12;i++){
        if(io_.read(address_,data,count))return true;
        io_.pauseMs(1); // bounded wait: SPI transfer may still be in progress
    }
    return false;
}
bool Sc18is606Port::begin(){
    ready_=false;
    if(address_<0x28||address_>0x2f)return false;
    const uint8_t id=0xfe; // read version, no SPI/CE activity
    uint8_t version[16]={};
    static const char EXPECTED[]="SC18IS606";
    if(!command(&id,1,true)||!fetch(version,sizeof(version))||
       memcmp(version,EXPECTED,sizeof(EXPECTED)-1)!=0)return false;
    // SPI mode 0, MSB first, 455kHz (conservative 0x01; NOT ESP TF SPI).
    const uint8_t spi[]={0xf0,0x01};
    // F4 GPIO latch low BEFORE F6/F7 enable push-pull to prevent CE glitch.
    const uint8_t ceLow[]={0xf4,0x00};
    // GPIO1 / SS1 becomes output; SS0 remains automatic nRF CSN.
    const uint8_t gpioEnable[]={0xf6,0x02};
    const uint8_t gpioOutput[]={0xf7,0x04}; // SS1[1:0]=01: push-pull
    const uint8_t gpioRead[]={0xf5};
    uint8_t state=0xff;
    if(!command(spi,sizeof(spi),true)||
       !command(ceLow,sizeof(ceLow),true)||
       !command(gpioEnable,sizeof(gpioEnable),true)||
       !command(gpioOutput,sizeof(gpioOutput),true)||
       !command(gpioRead,sizeof(gpioRead),true)||
       !fetch(&state,1)||((state&0x02U)!=0U)){
        disable();return false;
    }
    ready_=true;return true;
}
bool Sc18is606Port::ce(bool high){
    if(!ready_)return false;
    const uint8_t commandData[]={0xf4,(uint8_t)(high?0x02:0x00)};
    if(command(commandData,sizeof(commandData),true))return true;
    disable();return false;
}
bool Sc18is606Port::transfer(const uint8_t*tx,uint8_t*rx,size_t count){
    if(!ready_||!tx||!rx||count==0||count>MAX_TRANSFER)return false;
    uint8_t commandData[MAX_TRANSFER+1]={0x01};
    memcpy(commandData+1,tx,count);
    // Automatic SS0 select/de-select surrounds the complete SPI transfer.
    if(!command(commandData,count+1,false)||!fetch(rx,count)){
        disable();return false;
    }
    return true;
}
void Sc18is606Port::disable(){
    ready_=false;
    const uint8_t ceLow[]={0xf4,0x00};
    // Best effort only: if I2C is broken this cannot physically force CE low.
    (void)io_.write(address_,ceLow,sizeof(ceLow));
}
