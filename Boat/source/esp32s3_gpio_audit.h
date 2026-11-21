#pragma once
#include <stdint.h>
#include "mdd20a_direct_profile.h"
// ESP32-S3-WROOM-2 N32R16V conservative logic-pin audit.
// This is an extra software check, NOT a substitute for verifying the actual
// DevKitC-1 revision, headers, external circuit levels and boot conditions.
inline bool boatEsp32s3PinSafe3v3(int8_t gpio){
 if(gpio<0||gpio>48)return false;
 // Boot strapping, USB, UART0 console, octal flash/PSRAM, and 1.8V pads.
 switch(gpio){
 case 0:case 3:case 19:case 20:case 35:case 36:case 37:
 case 43:case 44:case 45:case 46:case 47:case 48:return false;
 default:return true;
 }
}
inline bool boatEsp32s3MotorPinAudit(const Mdd20aDirectProfile &p,
                                    uint64_t allowed){
 if(!mdd20aDirectProfileValid(p,allowed))return false;
 return boatEsp32s3PinSafe3v3(p.pwmLeft)&&
        boatEsp32s3PinSafe3v3(p.pwmRight)&&
        boatEsp32s3PinSafe3v3(p.dirLeft)&&
        boatEsp32s3PinSafe3v3(p.dirRight)&&
        boatEsp32s3PinSafe3v3(p.watchdogEdge)&&
        boatEsp32s3PinSafe3v3(p.permit);
}
