#pragma once
#include <stdint.h>
enum BoatBatteryBand : uint8_t { BOAT_BATTERY_UNKNOWN=0, BOAT_BATTERY_GREEN=1, BOAT_BATTERY_YELLOW=2, BOAT_BATTERY_RED=3, BOAT_BATTERY_CRITICAL=4 };
struct BoatBatterySnapshot { bool valid; uint16_t millivolts; uint8_t percent; BoatBatteryBand band; bool lowWarning; bool rthRequested; bool critical; };
inline uint8_t boatBatteryEstimatePercent(uint16_t mv){
 static const uint16_t v[]={9900,10200,10500,10800,11100,11400,11700,12000,12300,12600};
 static const uint8_t p[]={0,5,10,20,35,50,65,80,90,100};
 if(mv<=v[0])return 0;
 if(mv>=v[9])return 100;
 for(uint8_t i=1;i<10;++i)if(mv<=v[i])return (uint8_t)(p[i-1]+((uint32_t)(mv-v[i-1])*(p[i]-p[i-1]))/(v[i]-v[i-1]));
 return 100;
}
class BoatBatteryMonitor {
public:
 static const uint16_t YELLOW_MV=11400U,RTH_MV=10800U,CRITICAL_MV=10200U,IMMEDIATE_STOP_MV=9900U;
 static const uint32_t RTH_CONFIRM_MS=3000U,CRITICAL_CONFIRM_MS=1000U,MAX_SAMPLE_AGE_MS=1000U;
 BoatBatterySnapshot update(bool valid,uint16_t raw,uint32_t now){
  // A failed/implausible ADC conversion must fail closed on *this* tick.
  // Do not use the last good sample for motor authorization even if it is
  // younger than MAX_SAMPLE_AGE_MS. The latched alert state is preserved.
  if(!valid||raw<9000U||raw>13000U){sampleOK_=false;return snapshot(now);}
  sampleOK_=true;
  if(!initialized_){filtered_=raw;initialized_=true;}else filtered_=(uint16_t)(((uint32_t)filtered_*3U+raw+2U)/4U);
  last_=now;
  // An abrupt pack collapse must bypass the smoothing filter. In particular,
  // a 12.6V -> 9.8V step must STOP on the very first valid ADC reading.
  if(raw<=IMMEDIATE_STOP_MV){critical_=true;criticalTiming_=false;}
  else if(filtered_<=IMMEDIATE_STOP_MV){critical_=true;criticalTiming_=false;}
  else if(filtered_<=CRITICAL_MV){if(!criticalTiming_){criticalTiming_=true;criticalSince_=now;}if((uint32_t)(now-criticalSince_)>=CRITICAL_CONFIRM_MS)critical_=true;}
  else if(filtered_>CRITICAL_MV+200U)criticalTiming_=false;
  if(filtered_<=RTH_MV){if(!lowTiming_){lowTiming_=true;lowSince_=now;}if((uint32_t)(now-lowSince_)>=RTH_CONFIRM_MS)rth_=true;}
  else if(filtered_>RTH_MV+200U)lowTiming_=false;
  return snapshot(now);
 }
 BoatBatterySnapshot snapshot(uint32_t now)const{
  BoatBatterySnapshot s{};s.valid=sampleOK_&&initialized_&&(uint32_t)(now-last_)<=MAX_SAMPLE_AGE_MS;
  s.millivolts=s.valid?filtered_:0;s.percent=s.valid?boatBatteryEstimatePercent(filtered_):255;
  s.critical=critical_;s.rthRequested=rth_&&!critical_;s.lowWarning=s.valid&&(filtered_<=RTH_MV||rth_||critical_);
  if(critical_)s.band=BOAT_BATTERY_CRITICAL;else if(!s.valid)s.band=BOAT_BATTERY_UNKNOWN;
  else if(rth_||filtered_<=RTH_MV)s.band=BOAT_BATTERY_RED;else if(filtered_<=YELLOW_MV)s.band=BOAT_BATTERY_YELLOW;else s.band=BOAT_BATTERY_GREEN;
  return s;
 }
 void reset(){sampleOK_=initialized_=lowTiming_=criticalTiming_=rth_=critical_=false;filtered_=0;last_=lowSince_=criticalSince_=0;}
private:
 uint16_t filtered_=0;uint32_t last_=0,lowSince_=0,criticalSince_=0;
 bool sampleOK_=false,initialized_=false,lowTiming_=false,criticalTiming_=false,rth_=false,critical_=false;
};
