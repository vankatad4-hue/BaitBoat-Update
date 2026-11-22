#pragma once
#include <stdint.h>
#include "boat_battery_policy.h"
// R30B defense-in-depth for ONE shared 3S4P Boat battery.
// This is a pure, non-actuating software evidence check. It neither samples
// a sensor nor opens/closes the shared load bus; it cannot substitute for
// independent hardware STOP, branch protection or measured BMS capability.
namespace boat_r30b {
enum class EnergyEvidence : uint8_t {
 STOP_INVALID_OR_CRITICAL = 0, // never grant propulsion/coil authorization
 LOW_BATTERY = 1,             // mission outbound/release still disallowed
 NORMAL = 2
};
inline EnergyEvidence inspectBoatBattery(
    const BoatBatterySnapshot &snapshot,
    bool localElectricalHealth,
    bool sensorVoltageValid, uint16_t sensorRawMillivolts,
    uint32_t measuredAtMs, uint32_t nowMs) {
 // Require an actual, fresh, plausible LOCAL INA231 conversion. Last-known
 // voltage, RF telemetry, invalid DMM calibration or a 0V error sentinel are
 // not evidence to rearm STOP or enter radio-loss autonomous Return Home.
 if (!localElectricalHealth || !sensorVoltageValid ||
     static_cast<uint32_t>(nowMs-measuredAtMs) >
         BoatBatteryMonitor::MAX_SAMPLE_AGE_MS ||
     sensorRawMillivolts < 9000U || sensorRawMillivolts > 13000U ||
     !snapshot.valid || snapshot.critical ||
     snapshot.band == BOAT_BATTERY_UNKNOWN ||
     snapshot.band == BOAT_BATTERY_CRITICAL ||
     snapshot.millivolts == 0U)
   return EnergyEvidence::STOP_INVALID_OR_CRITICAL;
 // The existing Boat R79 mission supervisor decides whether conditional
 // RTH is safe; this check NEVER grants Home on its own.
 if (snapshot.rthRequested || snapshot.lowWarning ||
     snapshot.band == BOAT_BATTERY_RED)
   return EnergyEvidence::LOW_BATTERY;
 return EnergyEvidence::NORMAL;
}
inline bool canRequestConditionalHome(EnergyEvidence energy,
                                      bool homeStored,
                                      bool navigationSafe,
                                      bool noActiveRelease,
                                      bool localStopReleased) {
 return energy != EnergyEvidence::STOP_INVALID_OR_CRITICAL &&
        homeStored && navigationSafe && noActiveRelease &&
        localStopReleased;
}
} // namespace boat_r30b
