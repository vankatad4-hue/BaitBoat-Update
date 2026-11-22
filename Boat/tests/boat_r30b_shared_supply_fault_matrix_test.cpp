#include "../source/boat_shared_supply_fault_guard_r30b.h"
#include "../source/boat_r29_shared_bus_contract.h"
#include "../source/boat_radio_safety.h"
#include "../source/boat_physical_driver_adapter.h"
#include "../source/ina231_acquisition_r28.h"
#include <cassert>
#include <iostream>
using boat_r30b::EnergyEvidence;
using boat_r30b::inspectBoatBattery;
using boat_r30b::canRequestConditionalHome;

static EnergyEvidence evidence(const BoatBatterySnapshot &s,
   bool electricalHealth, bool sensorValid, uint16_t raw,
   uint32_t at, uint32_t now) {
 return inspectBoatBattery(s,electricalHealth,sensorValid,raw,at,now);
}

int main() {
 // One shared supply is the design choice, not a physical acceptance.
 static_assert(boat_r29::SHARED_3S_PACK,"one pack only");
 static_assert(!boat_r29::SEPARATE_MOTOR_SOLENOID_POWER,
               "no independently switched motor/coil supply");
 static_assert(!boat_r29::SHARED_BUS_PHYSICALLY_ACCEPTED,
               "no live power connection authorized");
 static_assert(!boat_r27::INA231_PHYSICAL_COMMISSIONED,
               "battery sensor not physically approved");

 BoatBatteryMonitor monitor;
 auto good=monitor.update(true,12600U,1000U);
 assert(good.valid && good.band==BOAT_BATTERY_GREEN);
 assert(evidence(good,true,true,12600U,1000U,1000U)==EnergyEvidence::NORMAL);
 // Even previously good telemetry must be denied once its age expires.
 assert(evidence(good,true,true,12600U,1000U,2001U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);
 assert(evidence(good,true,false,12600U,1000U,1001U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);
 assert(evidence(good,false,true,12600U,1000U,1001U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);
 assert(evidence(good,true,true,0U,1000U,1001U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);
 assert(evidence(good,true,true,13100U,1000U,1001U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);
 assert(evidence(good,true,true,12600U,1100U,1000U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL); // future timestamp
 auto none=monitor.update(false,0U,1200U);
 assert(!none.valid);
 assert(evidence(none,true,false,0U,1200U,1200U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);
 assert(!canRequestConditionalHome(EnergyEvidence::STOP_INVALID_OR_CRITICAL,
                                  true,true,true,true));

 // Valid LOW battery must never become a permission for unsafe Home.
 BoatBatteryMonitor redMonitor;
 auto low=redMonitor.update(true,10800U,4000U);
 assert(low.valid && low.band==BOAT_BATTERY_RED && low.lowWarning);
 assert(evidence(low,true,true,10800U,4000U,4000U)==EnergyEvidence::LOW_BATTERY);
 assert(!canRequestConditionalHome(EnergyEvidence::LOW_BATTERY,false,true,true,true));
 assert(!canRequestConditionalHome(EnergyEvidence::LOW_BATTERY,true,false,true,true));
 assert(!canRequestConditionalHome(EnergyEvidence::LOW_BATTERY,true,true,false,true));
 assert(!canRequestConditionalHome(EnergyEvidence::LOW_BATTERY,true,true,true,false));
 assert(canRequestConditionalHome(EnergyEvidence::LOW_BATTERY,true,true,true,true));
 // No unconditional Home command may be inferred merely because 10s elapsed.
 assert(!boatRadioLossRthDue(true,true,false,true,true,0U,10000U));
 assert(!boatRadioLossRthDue(true,true,true,false,true,0U,10000U));
 assert(!boatRadioLossRthDue(true,true,true,true,false,0U,10000U));
 assert(boatRadioLossRthDue(true,true,true,true,true,0U,10000U));
 low=redMonitor.update(true,10800U,7001U);
 assert(low.rthRequested && !low.critical);
 // The active release must have ended BEFORE software can consider Home.
 assert(!canRequestConditionalHome(evidence(low,true,true,10800U,7001U,7001U),
                                  true,true,false,true));

 // Shared bus hard collapse: bypass any smoothed high battery history.
 BoatBatteryMonitor collapse;
 assert(collapse.update(true,12600U,2500U).valid);
 const auto critical=collapse.update(true,9900U,2501U);
 assert(critical.critical && critical.band==BOAT_BATTERY_CRITICAL);
 assert(evidence(critical,true,true,9900U,2501U,2501U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);
 assert(!canRequestConditionalHome(evidence(critical,true,true,9900U,2501U,2501U),
                                  true,true,true,true));
 // Stop stays latched if voltage recovers until explicit rearm procedure.
 const auto rebound=collapse.update(true,12600U,2550U);
 assert(rebound.critical);
 assert(evidence(rebound,true,true,12600U,2550U,2550U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);

 // Roll-over valid-age arithmetic does not falsely freeze good telemetry.
 BoatBatteryMonitor wrap;
 auto fresh=wrap.update(true,12000U,0xfffffffeU);
 assert(fresh.valid);
 assert(evidence(fresh,true,true,12000U,0xfffffffeU,1U)==
        EnergyEvidence::NORMAL);
 assert(evidence(fresh,true,true,12000U,0xfffffffeU,1001U)==
        EnergyEvidence::STOP_INVALID_OR_CRITICAL);

 // A trapped MCU, NACKed sensor or absent independent contactor must not
 // cause real power writes. Neither the energy guard nor tests wire GPIO.
 BoatPhysicalDriverAdapter real;
 BoatPhysicalHal uncommissioned{};
 const uint16_t pulse[4]={100U,100U,100U,100U};
 assert(!real.begin(uncommissioned,pulse,false));
 assert(!boat_r29::exclusiveActuatorRequest({300,0,1}));
 assert(boat_r29::exclusiveActuatorRequest({0,0,-1}));
 std::cout<<"R30B common 3S brownout, freshness, critical STOP and Home prerequisites PASS (logic-only)\n";
 return 0;
}
