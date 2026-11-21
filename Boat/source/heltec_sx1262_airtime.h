#pragma once
#include <stdint.h>
// Standard explicit-header LoRa airtime, conservatively rounded up to ms.
// Coding rate cr=1..4 represents 4/5..4/8; returns 0 on invalid inputs.
inline uint32_t sx1262AirtimeMs(uint8_t payload,uint8_t sf,uint32_t bw,
                                 uint8_t cr,uint16_t preamble=8,bool crc=true){
 if(sf<7||sf>12||(bw!=125000&&bw!=250000&&bw!=500000)||
    cr<1||cr>4||preamble<6)return 0;
 const int de=((bw==125000&&sf>=11)||(bw==250000&&sf==12))?1:0;
 const int numerator=8*payload-4*sf+28+(crc?16:0);
 const int denominator=4*(sf-2*de);
 const int extra=numerator>0?(numerator+denominator-1)/denominator:0;
 const uint64_t quarterSymbols=(uint64_t)(preamble+8+extra*(cr+4))*4+17;
 const uint64_t us=(quarterSymbols*(1ULL<<sf)*1000000ULL+
                   (4ULL*bw-1ULL))/(4ULL*bw);
 return (uint32_t)((us+999ULL)/1000ULL);
}
// This provisional limiter is not a certified regional duty-cycle system:
// airtime history is not persistent across power loss. TX stays disabled by
// independent commissioning gates until legal and fail-safe timing reviewed.
class Sx1262DutyLimiter{
 struct Bucket{uint32_t minute,airtime;bool valid;};
 Bucket buckets_[60]{};
 uint32_t lastTx_=0,lastAir_=0;bool sent_=false;
 uint32_t used(uint32_t now)const{
  uint32_t sum=0,m=now/60000U;
  for(const auto &b:buckets_)if(b.valid&&(uint32_t)(m-b.minute)<60U)sum+=b.airtime;
  return sum;
 }
public:
 bool allows(uint32_t now,uint32_t ms)const{
  return ms>0&&ms<=36000U&&(!sent_||(uint32_t)(now-lastTx_)>=lastAir_*100U)
         &&used(now)+ms<=36000U;
 }
 bool reserve(uint32_t now,uint32_t ms){
  if(!allows(now,ms))return false;
  uint32_t minute=now/60000U;Bucket &b=buckets_[minute%60U];
  if(!b.valid||b.minute!=minute){b.minute=minute;b.airtime=0;b.valid=true;}
  b.airtime+=ms;lastTx_=now;lastAir_=ms;sent_=true;return true;
 }
};
