#pragma once
#include <stdint.h>
#include "esp32s3_gpio_audit.h"
// External 3.3V SPI microSD breakout, shared SPI clock/data with nRF24.
// No default CS assignment: check board header and RF CS deassertion first.
struct BoatSdModuleProfile {
 bool pinAuditSigned=false;
 int8_t sck=-1,mosi=-1,miso=-1,cs=-1;
 int8_t nrfCs=-1;
 uint32_t maxClockHz=4000000U;
 bool independentCsPullupsVerified=false;
 bool powerLevelVerified=false;
};
inline bool boatSdModuleProfileValid(const BoatSdModuleProfile &p){
 if(!p.pinAuditSigned||!p.independentCsPullupsVerified||!p.powerLevelVerified||
    p.maxClockHz<1000000U||p.maxClockHz>10000000U)return false;
 const int8_t pins[]={p.sck,p.mosi,p.miso,p.cs,p.nrfCs};
 for(int8_t pin:pins)if(!boatEsp32s3PinSafe3v3(pin))return false;
 for(unsigned i=0;i<5;++i)for(unsigned j=i+1;j<5;++j)
  if(pins[i]==pins[j])return false;
 return true;
}
