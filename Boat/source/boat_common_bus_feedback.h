// LEGACY, RETIRED: external STOP/watchdog/contactor/current-zero signoff
// is not part of the selected software-only project. This old contract is
// retained for historical tests and must not gate active firmware.
#pragma once
#include <stdint.h>
#include "boat_hardware_supervisor.h"
#include "boat_shared_bus_contract.h"

// R30C: electrical evidence schema for ONE common 3S actuator supply.
// MDD20A VM and EKM014 VH share a single distribution bus. There is no
// separate motor/coil high-current contactor or two independent power rails.
//
// This is a prospective, transport-neutral commissioning contract ONLY.
// It does not drive any pin, open a contactor, read a sensor or grant power.
// An ESP32 software watchdog, MCP23017 register read-back, BMS label,
// INA231 voltage measurement, or mirrored contact bits are NOT independent
// electrical feedback and must never be substituted for real hardware.
namespace boat_r30c {
static constexpr bool COMMON_BUS_HARDWARE_COMMISSIONED = false;
static constexpr uint32_t MAX_FEEDBACK_AGE_MS = 50U;
static constexpr uint16_t MAX_COMMAND_LEASE_MS = 100U;
static constexpr uint16_t MAX_COIL_PULSE_MS = 500U;

// These fields are evidence from *different physical safety functions*, not
// different 3S batteries. One common disconnect may open the entire actuator
// bus, provided its actual DC ratings and fault-off tests are signed.
struct CommonBusFeedback {
 bool stopCircuitHealthy = false;
 bool independentlySupervisedWatchdogHealthy = false;
 bool commonDisconnectClosedFeedback = false;
 bool commonDisconnectDcRatingSigned = false;
 bool commonDisconnectPowerOffTestSigned = false;
 bool coilOneHotPulseLimiterCommissioned = false;
 bool independentlyObservedCoilCurrentZero = false;
 bool independentlyObservedMotorCurrentZero = false;
 bool coilCurrentZero = false;
 bool motorCurrentZero = false;
 bool localBatteryHealthy = false;
 uint32_t sampledAtMs = 0;
};

enum class Review : uint8_t {
 OFF_REQUEST = 0,
 BAD_REQUEST,
 EXPIRED_FEEDBACK,
 STOP_OR_WATCHDOG_UNAVAILABLE,
 COMMON_DISCONNECT_UNQUALIFIED,
 BATTERY_UNTRUSTED,
 RELEASE_LIMITER_UNQUALIFIED,
 MISSING_CURRENT_ZERO,
 DESIGN_EVIDENCE_CONSISTENT
};

// Review is a *simulation/inspection* result. Even if it reports consistent
// evidence from a synthetic fixture, the permanent compile-time commissioning
// flag below denies physical output. It cannot be set by a radio command.
inline Review inspect(const CommonBusFeedback &e, const BoatActuatorOutput &r,
                      uint16_t leaseMs, uint16_t coilPulseMs, uint32_t nowMs) {
 if (!boat_r29::exclusiveActuatorRequest(r) ||
     leaseMs == 0U || leaseMs > MAX_COMMAND_LEASE_MS)
   return Review::BAD_REQUEST;
 if (r.left == 0 && r.right == 0 && r.releaseChannel < 0)
   return Review::OFF_REQUEST;
 if (static_cast<uint32_t>(nowMs-e.sampledAtMs) >= MAX_FEEDBACK_AGE_MS)
   return Review::EXPIRED_FEEDBACK;
 if (!e.stopCircuitHealthy || !e.independentlySupervisedWatchdogHealthy)
   return Review::STOP_OR_WATCHDOG_UNAVAILABLE;
 if (!e.commonDisconnectClosedFeedback ||
     !e.commonDisconnectDcRatingSigned ||
     !e.commonDisconnectPowerOffTestSigned)
   return Review::COMMON_DISCONNECT_UNQUALIFIED;
 if (!e.localBatteryHealthy)
   return Review::BATTERY_UNTRUSTED;
 // Both actuator classes need an independently qualified coil limiter:
 // without it, a stuck FET may remain ON while the common supply is alive.
 if (!e.coilOneHotPulseLimiterCommissioned)
   return Review::RELEASE_LIMITER_UNQUALIFIED;
 if (!e.independentlyObservedCoilCurrentZero ||
     !e.independentlyObservedMotorCurrentZero)
   return Review::MISSING_CURRENT_ZERO;
 if (r.releaseChannel >= 0) {
   if (coilPulseMs == 0U || coilPulseMs > MAX_COIL_PULSE_MS)
     return Review::BAD_REQUEST;
   // No motor may be spinning/driving while a release is started.
   if (!e.motorCurrentZero || !e.coilCurrentZero)
     return Review::MISSING_CURRENT_ZERO;
 } else {
   // During motor operation motorCurrentZero is naturally false; use the
   // independent *coil* current-zero evidence as the interlock instead.
   if (!e.coilCurrentZero)
     return Review::MISSING_CURRENT_ZERO;
 }
 return Review::DESIGN_EVIDENCE_CONSISTENT;
}

inline bool mayEnergizeActuators(const CommonBusFeedback &e,
                                const BoatActuatorOutput &r,
                                uint16_t leaseMs, uint16_t coilPulseMs,
                                uint32_t nowMs) {
 return COMMON_BUS_HARDWARE_COMMISSIONED &&
        inspect(e,r,leaseMs,coilPulseMs,nowMs) ==
            Review::DESIGN_EVIDENCE_CONSISTENT;
}
} // namespace boat_r30c
