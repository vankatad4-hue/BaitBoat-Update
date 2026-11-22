#pragma once
#include <stdint.h>

// R30H-5 GNSS UTC civil-time conversion, leap-year and freshness validation.
// No timezone/DST conversions, no NTP, no motor/STOP authority. The receiver's
// NMEA/UBX serial message completion time is NOT a PPS pulse.
// A seconds/ms UTC display may be useful; microsecond alignment is unproven.
namespace flytec_utc {
static constexpr uint64_t MIN_UNIX_MS=1577836800000ULL; // 2020-01-01 UTC
static constexpr uint64_t MAX_UNIX_MS=4102444800000ULL; // 2100-01-01 UTC
struct Sample {
 uint64_t unixMs=0;
 uint32_t localMs=0;
 uint32_t receiverTimeAccMs=0; // receiver tAcc; excludes UART/over-air delay
 bool valid=false;
};
inline bool leap(uint16_t y){return y%4U==0U && (y%100U!=0U||y%400U==0U);}
inline uint8_t daysInMonth(uint16_t y,uint8_t m){
 static const uint8_t days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
 return (m<1||m>12)?0:(uint8_t)(days[m-1]+(m==2&&leap(y)));
}
inline bool unixFromCivil(uint16_t year,uint8_t month,uint8_t day,
                          uint8_t hour,uint8_t minute,uint8_t second,
                          uint16_t millisecond,uint64_t &out) {
 if(year<2020||year>2099||day<1||day>daysInMonth(year,month)||
    hour>23||minute>59||second>59||millisecond>999)return false;
 // UTC epoch day count, signed intermediate to prevent unsigned wrap.
 int64_t days=0;
 for(uint16_t y=1970;y<year;++y)days+=leap(y)?366:365;
 for(uint8_t m=1;m<month;++m)days+=daysInMonth(year,m);
 days+=day-1;
 const uint64_t result=(((uint64_t)days*24ULL+hour)*60ULL+minute)*60000ULL+
                       (uint64_t)second*1000ULL+millisecond;
 if(result<MIN_UNIX_MS||result>=MAX_UNIX_MS)return false;
 out=result;return true;
}
inline bool utcFromNmeaDate(const char *date,uint32_t utcOfDayMs,
                           uint64_t &out){
 if(!date||utcOfDayMs>=86400000UL)return false;
 for(unsigned i=0;i<6;++i)if(date[i]<'0'||date[i]>'9')return false;
 if(date[6]!=0)return false;
 const uint8_t day=(uint8_t)((date[0]-'0')*10+(date[1]-'0'));
 const uint8_t month=(uint8_t)((date[2]-'0')*10+(date[3]-'0'));
 const uint16_t year=(uint16_t)(2000+(date[4]-'0')*10+(date[5]-'0'));
 return unixFromCivil(year,month,day,(uint8_t)(utcOfDayMs/3600000UL),
                      (uint8_t)((utcOfDayMs/60000UL)%60U),
                      (uint8_t)((utcOfDayMs/1000UL)%60U),
                      (uint16_t)(utcOfDayMs%1000U),out);
}
// UBX NAV-PVT nano is signed relative to UTC second, can be negative.
inline bool utcFromUbx(uint16_t year,uint8_t month,uint8_t day,
                       uint8_t hour,uint8_t minute,uint8_t second,
                       int32_t nanosecond,uint32_t accuracyNs,uint8_t validity,
                       uint64_t &unixMs,uint32_t &accuracyMs) {
 if((validity&0x07U)!=0x07U||accuracyNs==0||accuracyNs>250000000U||
    nanosecond<=-1000000000||nanosecond>=1000000000)return false;
 uint64_t secMs=0;
 if(!unixFromCivil(year,month,day,hour,minute,second,0,secMs))return false;
 // Quantize to nearest whole ms: conservatively reject large rounding risk.
 const int64_t raw=(int64_t)secMs+(int64_t)nanosecond/1000000LL;
 if(raw<(int64_t)MIN_UNIX_MS||raw>=(int64_t)MAX_UNIX_MS)return false;
 unixMs=(uint64_t)raw;
 accuracyMs=(accuracyNs+999999U)/1000000U;
 return true;
}
inline bool fresh(const Sample &s,uint32_t now,uint32_t maxAgeMs=1500U){
 return s.valid && s.unixMs>=MIN_UNIX_MS&&s.unixMs<MAX_UNIX_MS&&
        (uint32_t)(now-s.localMs)<=maxAgeMs;
}
inline bool extrapolate(const Sample &s,uint32_t now,uint64_t &utc){
 if(!fresh(s,now))return false;
 utc=s.unixMs+(uint32_t)(now-s.localMs);
 return utc<MAX_UNIX_MS;
}
}
