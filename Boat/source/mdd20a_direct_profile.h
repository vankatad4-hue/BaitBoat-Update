#pragma once
#include <stdint.h>
// Direct Heltec PWM/DIR with non-programmable peripheral expansion if required.
// No default assignments: validate the exact board pinout before enabling.
struct Mdd20aDirectProfile{
 bool pinAuditSigned;
 int8_t pwmLeft,pwmRight,dirLeft,dirRight,watchdogEdge,permit;
};
// Caller supplies the audited set of accessible, output-capable free GPIOs.
// A bit in allowed must NOT belong to a boot strap, flash, GNSS, radio,
// USB, display, sensor bus, or any other reserved function.
inline bool mdd20aDirectProfileValid(const Mdd20aDirectProfile &p,uint64_t allowed){
 if(!p.pinAuditSigned)return false;
 const int8_t pins[]={p.pwmLeft,p.pwmRight,p.dirLeft,p.dirRight,p.watchdogEdge,p.permit};
 uint64_t used=0;
 for(int8_t n:pins){
   if(n<0||n>48)return false;
   const uint64_t bit=uint64_t(1)<<n;
   if(!(allowed&bit)||(used&bit))return false;
   used|=bit;
 }
 return true;
}
inline Mdd20aDirectProfile mdd20aUnassignedProfile(){return {false,-1,-1,-1,-1,-1,-1};}
