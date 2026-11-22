#include "../source/boat_ads1115_bench_math.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

static int16_t countsForPackVolts(double packVolts){
 const double adcInputMv=boatBatteryDividerInputVolts(packVolts)*1000.0;
 return (int16_t)std::lround(adcInputMv/0.125);
}
int main(){
 static_assert(BOAT_ADS1115_CONFIG_AIN0_SINGLE_128SPS_4V096==0xC383,
               "ADS1115 AIN0/single-shot/4.096V/128SPS config changed");
 assert(std::fabs(BOAT_BATTERY_DIVIDER_RATIO-5.54545454545)<0.000001);
 assert(boatBatteryDividerInputVolts(12.6)<3.0);
 // A fully charged 3S battery must be accepted as an *uncalibrated*
 // diagnostic value, NOT treated as proof of battery-health authorization.
 const auto full=boatAdcBenchDecode(countsForPackVolts(12.6));
 assert(full.plausible());
 assert(full.inputMilliVolts>=2271&&full.inputMilliVolts<=2274);
 assert(full.estimatedPackMilliVolts>=12595&&full.estimatedPackMilliVolts<=12605);
 assert(std::strcmp(boatAdcBenchReasonText(full.reason),"PLAUSIBLE_UNCALIBRATED")==0);
 const auto red=boatAdcBenchDecode(countsForPackVolts(10.8));
 assert(red.plausible()&&red.estimatedPackMilliVolts>=10795&&red.estimatedPackMilliVolts<=10805);
 const auto zero=boatAdcBenchDecode(0);
 assert(!zero.plausible()&&zero.reason==BOAT_ADC_BENCH_NONPOSITIVE_RAW);
 assert(!boatAdcBenchDecode(-1).plausible());
 const auto outOfRange=boatAdcBenchDecode(24001);
 assert(!outOfRange.plausible()&&outOfRange.reason==BOAT_ADC_BENCH_INPUT_RANGE);
 const auto undervoltage=boatAdcBenchDecode(countsForPackVolts(8.5));
 assert(!undervoltage.plausible()&&undervoltage.reason==BOAT_ADC_BENCH_PACK_RANGE);
 const auto overvoltage=boatAdcBenchDecode(countsForPackVolts(13.2));
 assert(!overvoltage.plausible()&&overvoltage.reason==BOAT_ADC_BENCH_PACK_RANGE);
 std::cout<<"R22 ADS1115 bench-only raw decoder passed\n";
}
