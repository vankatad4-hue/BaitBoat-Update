#pragma once
#include <stdint.h>

// R30H-5 authenticated, nRF-only UTC display/RTC beacon, TELEMETRY subtype 9.
// Payload: [9,1,unix_seconds u32 LE, milliseconds u16 LE] = 8 bytes.
// No remote control/STOP authority; no ACK, link freshness, or RTH age change.
// One-way packet latency and GNSS UART parsing time are NOT calibrated by PPS/RTT.
namespace flytec_time {
static constexpr uint8_t SUBTYPE=9;
static constexpr uint8_t VERSION=1;
static constexpr uint64_t MIN_MS=1577836800000ULL;
static constexpr uint64_t MAX_MS=4102444800000ULL;
static constexpr uint32_t SOURCE_MAX_AGE_MS=1500U;
static constexpr uint32_t DISCIPLINE_MAX_AGE_MS=20000U;
static constexpr uint32_t RTC_SET_INTERVAL_MS=60000U;

inline bool plausible(uint64_t value){return value>=MIN_MS && value<MAX_MS;}
inline bool encode(uint64_t unixMs,uint8_t out[8]){
 if(!out||!plausible(unixMs))return false;
 const uint32_t seconds=(uint32_t)(unixMs/1000ULL);
 const uint16_t rem=(uint16_t)(unixMs%1000ULL);
 out[0]=SUBTYPE;out[1]=VERSION;
 out[2]=(uint8_t)seconds;out[3]=(uint8_t)(seconds>>8);
 out[4]=(uint8_t)(seconds>>16);out[5]=(uint8_t)(seconds>>24);
 out[6]=(uint8_t)rem;out[7]=(uint8_t)(rem>>8);
 return true;
}
inline bool decode(const uint8_t p[8],uint64_t &out){
 if(!p||p[0]!=SUBTYPE||p[1]!=VERSION)return false;
 const uint32_t secs=(uint32_t)p[2]|((uint32_t)p[3]<<8)|
                     ((uint32_t)p[4]<<16)|((uint32_t)p[5]<<24);
 const uint16_t ms=(uint16_t)p[6]|((uint16_t)p[7]<<8);
 if(ms>=1000U)return false;
 const uint64_t value=(uint64_t)secs*1000ULL+ms;
 if(!plausible(value))return false;
 out=value;return true;
}
class Discipline {
public:
 void reset(){qualified_=false;haveFirst_=false;count_=0;lastLocal_=0;
              lastUnix_=0;rtcSet_=false;lastRtcSetMs_=0;}
 // Call only after checked GNSS PVT or HMAC + sequence + nRF source/link.
 // nowMs is local millis (monotonic, NEVER changed); UTC drift/jumps rejected.
 bool observe(uint64_t utcMs,uint32_t nowMs){
  if(!plausible(utcMs))return false;
  if(haveFirst_){
   const uint32_t age=(uint32_t)(nowMs-lastLocal_);
   if(age<=10000U){
    const int64_t residual=(int64_t)utcMs-(int64_t)(lastUnix_+age);
    if(residual < -2000LL || residual > 2000LL) {
     // Contradictory GNSS/RF readings break qualification, never step
     // wall-clock backwards on one outlier.
     reset();
     return false;
    }
   }else {qualified_=false;count_=0;}
  }
  lastUnix_=utcMs;lastLocal_=nowMs;haveFirst_=true;
  if(count_<2U)++count_;
  qualified_=count_>=2U;
  return true;
 }
 bool utcNow(uint32_t nowMs,uint64_t &out)const{
  if(!qualified_||!haveFirst_)return false;
  const uint32_t age=(uint32_t)(nowMs-lastLocal_);
  if(age>DISCIPLINE_MAX_AGE_MS)return false;
  out=lastUnix_+age;return plausible(out);
 }
 bool shouldSetRtc(uint32_t nowMs)const{
  uint64_t unused=0;
  return utcNow(nowMs,unused)&&(!rtcSet_||
         (uint32_t)(nowMs-lastRtcSetMs_)>=RTC_SET_INTERVAL_MS);
 }
 void markRtcSet(uint32_t nowMs){lastRtcSetMs_=nowMs;rtcSet_=true;}
 // Read-only quality: local monotonic age of last source observation.
 // No bound on UART transmission/RF transit delay without PPS/RTT.
 uint32_t lastSampleAgeMs(uint32_t nowMs)const{
  return haveFirst_?(uint32_t)(nowMs-lastLocal_):0xFFFFFFFFU;
 }
 bool qualified()const{return qualified_;}
private:
 uint64_t lastUnix_=0;
 uint32_t lastLocal_=0,lastRtcSetMs_=0;
 uint8_t count_=0;
 bool haveFirst_=false,qualified_=false,rtcSet_=false;
};
}
