#pragma once
#include <stdint.h>
#include <cmath>
#include "boat_hardware_supervisor.h"
// R29 selected wiring topology, not an independent power cutoff.
// ONE existing 3S4P pack feeds one common high-current 3S bus; both
// Cytron MDD20A VM and Electrokit EKM014 VH are parallel loads on it.
// No separate motor/solenoid battery or individually switched power rail.
// 5V EKM014 VL and the MCU/sensor/radio regulators remain logic supplies.
namespace boat_r29 {
static constexpr bool SHARED_3S_PACK = true;
static constexpr bool SEPARATE_MOTOR_SOLENOID_POWER = false;
static constexpr bool SHARED_BUS_PHYSICALLY_ACCEPTED = false;
static constexpr uint16_t PACK_MAX_MILLIVOLTS = 12600U;
static constexpr uint16_t PACK_NOMINAL_MILLIVOLTS = 11100U;
static constexpr uint16_t BMS_LABEL_CURRENT_AMPS = 20U;
// A pure request-shape check, reused by the existing physical output
// driver only to DENY conflicting requests. It cannot grant authorization.
inline bool exclusiveActuatorRequest(const BoatActuatorOutput &r) {
 return r.left >= -1000 && r.left <= 1000 &&
        r.right >= -1000 && r.right <= 1000 &&
        r.releaseChannel >= -1 && r.releaseChannel <= 3 &&
        (r.releaseChannel < 0 || (r.left == 0 && r.right == 0));
}
// Engineering estimation only. No measured motor stall or coil pulse
// currents exist yet: passing this function is NOT evidence of safe wiring.
struct EstimatedLoad {
 double leftMotorAmps;
 double rightMotorAmps;
 double oneSolenoidAmps;
 double otherPackAmps;
};
inline bool syntheticCurrentWithinBmsNameplate(const EstimatedLoad &x) {
 if (!std::isfinite(x.leftMotorAmps) || !std::isfinite(x.rightMotorAmps) ||
     !std::isfinite(x.oneSolenoidAmps) || !std::isfinite(x.otherPackAmps) ||
     x.leftMotorAmps < 0.0 || x.rightMotorAmps < 0.0 ||
     x.oneSolenoidAmps < 0.0 || x.otherPackAmps < 0.0 ||
     (x.oneSolenoidAmps > 0.0 &&
       (x.leftMotorAmps > 0.0 || x.rightMotorAmps > 0.0)))return false;
 return x.leftMotorAmps + x.rightMotorAmps +
        x.oneSolenoidAmps + x.otherPackAmps <= BMS_LABEL_CURRENT_AMPS;
}
} // namespace boat_r29
