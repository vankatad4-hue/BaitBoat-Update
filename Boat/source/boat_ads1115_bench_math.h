#pragma once
#include <stdint.h>
#include <cmath>
#include "boat_battery_divider.h"

// R22 bench-only decoded ADS1115 counts. NEVER a propulsion/mission permission.
// The sketch using this helper must remain isolated from the Boat application.
// ADS1115 PGA +/-4.096V => 125 microvolts per signed count.
enum BoatAdcBenchReason : uint8_t {
  BOAT_ADC_BENCH_PLAUSIBLE_UNCALIBRATED=0,
  BOAT_ADC_BENCH_NONPOSITIVE_RAW=1,
  BOAT_ADC_BENCH_INPUT_RANGE=2,
  BOAT_ADC_BENCH_PACK_RANGE=3
};
struct BoatAdcBenchReading {
  int16_t raw;
  uint16_t inputMilliVolts;
  uint16_t estimatedPackMilliVolts;
  BoatAdcBenchReason reason;
  bool plausible() const { return reason==BOAT_ADC_BENCH_PLAUSIBLE_UNCALIBRATED; }
};
static constexpr uint16_t BOAT_ADS1115_CONFIG_AIN0_SINGLE_128SPS_4V096 = 0xC383;
// This is a *diagnostic plausibility check*, not an ADS1115 identification test.
inline BoatAdcBenchReading boatAdcBenchDecode(int16_t raw) {
  BoatAdcBenchReading r={raw,0,0,BOAT_ADC_BENCH_NONPOSITIVE_RAW};
  if(raw<=0)return r;
  const uint32_t uv=(uint32_t)raw*125U;
  r.inputMilliVolts=(uint16_t)((uv+500U)/1000U);
  if(uv>3000000U){r.reason=BOAT_ADC_BENCH_INPUT_RANGE;return r;}
  const double packMv=((double)uv/1000.0)*BOAT_BATTERY_DIVIDER_RATIO;
  if(!std::isfinite(packMv)||packMv<9000.0||packMv>13000.0){
    r.reason=BOAT_ADC_BENCH_PACK_RANGE;return r;
  }
  r.estimatedPackMilliVolts=(uint16_t)std::lround(packMv);
  r.reason=BOAT_ADC_BENCH_PLAUSIBLE_UNCALIBRATED;
  return r;
}
inline const char *boatAdcBenchReasonText(BoatAdcBenchReason reason){
  switch(reason){
    case BOAT_ADC_BENCH_PLAUSIBLE_UNCALIBRATED:return "PLAUSIBLE_UNCALIBRATED";
    case BOAT_ADC_BENCH_NONPOSITIVE_RAW:return "INVALID_RAW";
    case BOAT_ADC_BENCH_INPUT_RANGE:return "INPUT_RANGE";
    default:return "PACK_RANGE";
  }
}
