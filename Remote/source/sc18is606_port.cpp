#include "sc18is606_port.h"
#include <string.h>

bool Sc18is606Port::command(const uint8_t*data,size_t count,bool retry){
    if(!data||count==0||count>MAX_TRANSFER+1)return false;
    // SC18IS606 can NACK its I2C address while it processes a previous
    // request. Never replay a non-idempotent SPI command after an ambiguous
    // write result: its 32-byte payload might already have transmitted.
    const unsigned attempts=retry?5U:1U;
    const uint32_t start=io_.tickMs();
    for(unsigned i=0;i<attempts;i++){
        const bool ok=io_.write(address_,data,count);
        const uint32_t elapsed=(uint32_t)(io_.tickMs()-start);
        if(elapsed>OPERATION_BUDGET_MS)return false;
        if(ok)return true;
        if(!retry||i+1U>=attempts||elapsed>=OPERATION_BUDGET_MS)break;
        io_.pauseMs(1);
    }
    return false;
}
bool Sc18is606Port::fetch(uint8_t*data,size_t count){
    if(!data||count==0||count>MAX_TRANSFER)return false;
    const uint32_t start=io_.tickMs();
    for(unsigned i=0;i<12U;i++){
        const bool ok=io_.read(address_,data,count);
        const uint32_t elapsed=(uint32_t)(io_.tickMs()-start);
        // Reject a late successful buffer read. It may already violate the
        // expected STOP priority/heartbeat service interval.
        if(elapsed>OPERATION_BUDGET_MS)return false;
        if(ok)return true;
        if(elapsed>=OPERATION_BUDGET_MS||i+1U>=12U)break;
        io_.pauseMs(1);
    }
    return false;
}
bool Sc18is606Port::begin(){
    ready_=false;
    if(address_<0x28||address_>0x2f||!ce_.beginLow())return false;
    const uint8_t id=0xfe; // read version, no SPI/CE activity
    uint8_t version[16]={};
    static const char EXPECTED[]="SC18IS606";
    if(!command(&id,1,true)||!fetch(version,sizeof(version))||
       memcmp(version,EXPECTED,sizeof(EXPECTED)-1)!=0){
        disable();return false;
    }
    // NXP SS2 comes up as an inactive SPI select (likely driven HIGH).
    // Do NOT tie it directly to the active-high nRF CE. CE is instead
    // initialized low via existing MCP23017 GPA0 before any bridge setup.
    // SS0 is nRF CSN, SS1 remains the EVB EEPROM, SS2 is unconnected.
    // SPI mode 0, MSB first, conservative 455 kHz, separate from TF.
    const uint8_t spi[]={0xf0,0x01};
    if(!command(spi,sizeof(spi),true)){
        disable();return false;
    }
    ready_=true;return true;
}
bool Sc18is606Port::ce(bool high){
    if(!ready_)return false;
    if(ce_.write(high))return true;
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
    ce_.disable(); // best effort only; not an independent hardware cutoff
}
