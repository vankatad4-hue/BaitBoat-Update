#pragma once
#include <stdint.h>
#include "boat_common_bus_feedback_r30c.h"
#include "ina231_voltage_contract_r27.h"

// R30D READ-ONLY diagnostic adapter for the selected ONE common 3S bus.
// It has NO GPIO, Arduino, motor, coil, contactor or watchdog output APIs.
// An actual reader remains UNSELECTED until compatible independent hardware
// and the correct module revision/pin levels are physically verified.
namespace boat_r30d {
static constexpr bool ACTUAL_READER_COMMISSIONED = false;
static constexpr uint32_t MAX_BATTERY_AGE_MS = 1000U;

// Assigned from *documented distinct external sources*, not ESP output
// register read-back or synthetic radio acknowledgements. IDs catch an
// obvious repeated input source, but distinct IDs NEVER prove independence.
struct Observation {
 bool asserted = false;
 uint32_t acquiredAtMs = 0;
 uint8_t sourceId = 0;    // zero means unassigned/unverified input
};
struct BatteryObservation {
 bool valid = false;
 uint16_t millivolts = 0;
 uint32_t acquiredAtMs = 0;
 uint8_t sourceId = 0;
};
// Reader input values are raw observations, NOT a commissioning signature.
// STOP, watchdog, disconnect, limiter and current sensors can be ON/OFF;
// false values remain valid observations and are reported fail-closed.
struct RawFrame {
 Observation stop;
 Observation watchdog;
 Observation commonDisconnectAux;
 Observation coilPulseLimiter;
 Observation coilCurrentZero;
 Observation motorCurrentZero;
 BatteryObservation battery;
};
class ReadOnlySource {
public:
 virtual ~ReadOnlySource() {}
 virtual bool read(RawFrame &out) = 0;
 // Deliberately no write(), setPin(), permit(), clearStop(), or pulse().
};
enum class Result : uint8_t {
 NEVER_READ,
 OK_DIAGNOSTIC_ONLY,
 IO_FAILURE,
 SOURCE_UNASSIGNED,
 DUPLICATE_SOURCE,
 STALE_SAFETY,
 STALE_BATTERY,
 INVALID_BATTERY
};
struct Capture {
 Result result = Result::NEVER_READ;
 boat_r30c::CommonBusFeedback feedback{};
 RawFrame raw{}; // visible only to diagnostics; never a signed safety contact
 uint32_t capturedAtMs = 0;
 bool observed = false;
 // No physical commissioning can be inferred from an observation frame.
 bool canAuthorizeActuators = false;
};

class ReadOnlyCommonBusPort {
public:
 Capture sample(ReadOnlySource &reader, uint32_t nowMs) {
  latest_ = Capture{}; // invalidates previously displayed observations
  latest_.capturedAtMs=nowMs;
  RawFrame frame{};
  if (!reader.read(frame)) return deny(Result::IO_FAILURE);
  const Observation *signals[6] = {
   &frame.stop,&frame.watchdog,&frame.commonDisconnectAux,
   &frame.coilPulseLimiter,&frame.coilCurrentZero,&frame.motorCurrentZero
  };
  uint8_t ids[7] = {};
  for (unsigned i=0;i<6;++i) ids[i]=signals[i]->sourceId;
  ids[6]=frame.battery.sourceId;
  for (unsigned i=0;i<7;++i) {
   if (!ids[i]) return deny(Result::SOURCE_UNASSIGNED);
   for (unsigned k=0;k<i;++k)
    if (ids[i]==ids[k]) return deny(Result::DUPLICATE_SOURCE);
  }
  // Modular millis subtraction handles rollover and refuses future timestamps
  // (other than an actual 32-bit wrap with a small elapsed duration).
  uint32_t oldestAge=0;
  for (const Observation *s : signals) {
   const uint32_t age=static_cast<uint32_t>(nowMs-s->acquiredAtMs);
   if (age>=boat_r30c::MAX_FEEDBACK_AGE_MS)
    return deny(Result::STALE_SAFETY);
   if (age>oldestAge) oldestAge=age;
  }
  if (static_cast<uint32_t>(nowMs-frame.battery.acquiredAtMs) >
      MAX_BATTERY_AGE_MS) return deny(Result::STALE_BATTERY);
  if (!frame.battery.valid || frame.battery.millivolts<9000U ||
      frame.battery.millivolts>13000U)
    return deny(Result::INVALID_BATTERY);

  latest_.raw=frame; // preserve limiter observation without commissioning it
  auto &f=latest_.feedback;
  f.stopCircuitHealthy=frame.stop.asserted;
  f.independentlySupervisedWatchdogHealthy=frame.watchdog.asserted;
  f.commonDisconnectClosedFeedback=frame.commonDisconnectAux.asserted;
  f.coilOneHotPulseLimiterCommissioned=false; // requires REAL signed hardware
  f.coilCurrentZero=frame.coilCurrentZero.asserted;
  f.motorCurrentZero=frame.motorCurrentZero.asserted;
  f.localBatteryHealthy=boat_r27::INA231_PHYSICAL_COMMISSIONED &&
                         frame.battery.valid;
  // Read-only sampling NEVER sets any of the following certified facts,
  // irrespective of what a mock input reader reported:
  f.commonDisconnectDcRatingSigned=false;
  f.commonDisconnectPowerOffTestSigned=false;
  f.independentlyObservedCoilCurrentZero=false;
  f.independentlyObservedMotorCurrentZero=false;
  f.sampledAtMs=nowMs-oldestAge;
  latest_.observed=true;
  latest_.result=Result::OK_DIAGNOSTIC_ONLY;
  return latest_;
 }
 Capture snapshot(uint32_t nowMs) const {
  Capture result=latest_;
  if (result.observed &&
      static_cast<uint32_t>(nowMs-result.feedback.sampledAtMs) >=
          boat_r30c::MAX_FEEDBACK_AGE_MS) {
   result=Capture{};
   result.result=Result::STALE_SAFETY;
   result.capturedAtMs=nowMs;
  }
  return result;
 }
 void clear(){latest_=Capture{};}
private:
 Capture deny(Result why){
  latest_.result=why;
  latest_.observed=false;
  latest_.feedback=boat_r30c::CommonBusFeedback{};
  latest_.canAuthorizeActuators=false;
  return latest_;
 }
 Capture latest_{};
};
} // namespace boat_r30d
