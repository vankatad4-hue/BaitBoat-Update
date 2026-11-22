#include "../source/ina226_voltage_candidate_r26.h"
#include "../source/boat_battery_policy.h"
#include <cassert>
#include <iostream>
using namespace boat_r26;
static VoltageEvidence fixture(uint16_t raw, uint32_t at=100) {
 VoltageEvidence e{};
 e.moduleRevisionSigned=true;
 e.busPinoutAndAddressSigned=true;
 e.i2cReadOk=true;
 e.newConversionObserved=true;
 e.dmmCalibrationSigned=true;
 e.profileCommissioned=true;
 e.address=INA226_CANDIDATE_ADDR;
 e.manufacturerId=0x5449;
 e.dieId=0x2260;
 e.rawBusRegister=raw;
 e.acquiredAtMs=at;
 return e;
}
int main() {
 uint16_t mv=0xffff;
 static_assert(!INA226_HARDWARE_SELECTED,"R26 alternate must never activate hardware");
 assert(REG_BUS_VOLTAGE==0x02 && REG_MANUFACTURER==0xFE && REG_DIE_ID==0xFF);
 assert(fromBigEndian(0x27,0x60)==0x2760);
 assert(idMatches(0x5449,0x2260));assert(idMatches(0x5449,0x2261));
 assert(!idMatches(0x5448,0x2260));assert(!idMatches(0x5449,0x2270));
 assert(candidateRegisterMillivolts(10080)==12600);
 assert(candidateRegisterMillivolts(9120)==11400);
 assert(candidateRegisterMillivolts(8640)==10800);
 assert(candidateRegisterMillivolts(8160)==10200);
 assert(candidateRegisterMillivolts(7920)==9900);
 // Exercise arithmetic/identity/freshness independently of live selection.
 auto accepted=fixture(10080);
 assert(decodeEvidenceOnly(accepted,100,mv) && mv==12600);
 accepted.dieId=0x2261;
 assert(decodeEvidenceOnly(accepted,100,mv) && mv==12600);
 accepted.dieId=0x2270;
 assert(!decodeEvidenceOnly(accepted,100,mv) && mv==0);
 accepted.dieId=0x2260;accepted.newConversionObserved=false;
 assert(!decodeEvidenceOnly(accepted,100,mv));
 accepted.newConversionObserved=true;
 assert(!decodeEvidenceOnly(accepted,1101,mv));
 accepted.acquiredAtMs=100;accepted.rawBusRegister=0x8001;
 assert(!decodeEvidenceOnly(accepted,100,mv));
 accepted.rawBusRegister=0;
 assert(!decodeEvidenceOnly(accepted,100,mv));
 // R79 policy interface: hypothetical decoded pack mV uses the same critical
 // threshold, not ADS1115 ADC-count units. No physical sensor selected.
 BoatBatteryMonitor policy;
 auto nominal=fixture(10080);
 assert(decodeEvidenceOnly(nominal,200,mv));
 auto state=policy.update(true,mv,200);
 assert(state.valid && state.band==BOAT_BATTERY_GREEN);
 auto low=fixture(7920);
 assert(decodeEvidenceOnly(low,250,mv) && mv==9900);
 state=policy.update(true,mv,250);
 assert(state.critical && state.band==BOAT_BATTERY_CRITICAL);
 // No combination of mock flags can authorize this unselected model.
 auto e=fixture(10080);
 assert(!readOnlyDecode(e,100,mv) && mv==0);
 e.moduleRevisionSigned=false;
 assert(!readOnlyDecode(e,100,mv));
 e.moduleRevisionSigned=true;e.newConversionObserved=false;
 assert(!readOnlyDecode(e,100,mv));
 e.newConversionObserved=true;e.i2cReadOk=false;
 assert(!readOnlyDecode(e,100,mv));
 e.i2cReadOk=true;e.address=0x48;
 assert(!readOnlyDecode(e,100,mv));
 e.address=INA226_CANDIDATE_ADDR;e.dieId=0x2270;
 assert(!readOnlyDecode(e,100,mv));
 e.dieId=0x2261;e.rawBusRegister=0x8001;
 assert(!readOnlyDecode(e,100,mv));
 e.rawBusRegister=10080;
 assert(!readOnlyDecode(e,1101,mv));
 std::cout<<"R26 INA226 candidate identity, millivolt codec and permanently disabled hardware gate PASS\n";
}
