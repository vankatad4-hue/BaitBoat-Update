#include "../source/ina231_voltage_contract.h"
#include "../source/boat_battery_policy.h"
#include <cassert>
#include <iostream>
using namespace boat_r27;
static VoltageOnlyEvidence fixture(uint16_t raw=10080) {
 VoltageOnlyEvidence e{};
 e.pcbAndBusContinuityVerified=true;
 e.shuntNotInPowerPath=true;
 e.i2cDomainVerified=true;
 e.dmmCalibrationSigned=true;
 e.newConversionConfirmed=true;
 e.readAndConfigVerified=true;
 e.applicationProfileCommissioned=true;
 e.actualAddress=I2C_CANDIDATE_ADDRESS;
 e.configurationReadback=CONFIG_VBUS_CONTINUOUS;
 e.rawBusVoltage=raw;
 e.conversionAtMs=100;
 return e;
}
int main(){
 static_assert(INA231_MODULE_SELECTED,"R27 module chosen for procurement");
 static_assert(!INA231_PHYSICAL_COMMISSIONED,"R27 physical approvals remain unsigned");
 static_assert((CONFIG_VBUS_CONTINUOUS&0x7U)==6U,"BUS only (not shunt) conversion");
 assert(REG_CONFIG==0&&REG_BUS==2&&REG_MASK==6&&I2C_CANDIDATE_ADDRESS==0x40);
 assert(rawToMillivolts(10080)==12600);
 assert(rawToMillivolts(9120)==11400);
 assert(rawToMillivolts(8640)==10800);
 assert(rawToMillivolts(8160)==10200);
 assert(rawToMillivolts(7920)==9900);
 auto e=fixture();uint16_t mv=0xffff;
 assert(decodeBenchFixture(e,100,mv)&&mv==12600);
 assert(!readForBoatSafety(e,100,mv)&&mv==0); // always blocked
 e.shuntNotInPowerPath=false;
 assert(!decodeBenchFixture(e,100,mv)&&mv==0);
 e=fixture();e.newConversionConfirmed=false;
 assert(!decodeBenchFixture(e,100,mv));
 e=fixture();e.actualAddress=0x48;
 assert(!decodeBenchFixture(e,100,mv));
 e=fixture();e.configurationReadback=0x4127; // shunt + bus = wrong mode
 assert(!decodeBenchFixture(e,100,mv));
 e=fixture();e.rawBusVoltage=0x8001;assert(!decodeBenchFixture(e,100,mv));
 e=fixture(10080);assert(!decodeBenchFixture(e,1101,mv));
 e=fixture(0);assert(!decodeBenchFixture(e,100,mv));
 e=fixture(10080);e.pcbAndBusContinuityVerified=false;
 assert(!decodeBenchFixture(e,100,mv));
 e=fixture(10080);e.applicationProfileCommissioned=false;
 assert(!decodeBenchFixture(e,100,mv));
 // Demonstrate compatibility of hypothetical VBUS voltage millivolts with R79.
 // This is a software fixture, not a physically commissioned sensor path.
 BoatBatteryMonitor policy;
 e=fixture(10080);assert(decodeBenchFixture(e,100,mv));
 auto s=policy.update(true,mv,100);
 assert(s.valid&&s.band==BOAT_BATTERY_GREEN);
 e=fixture(7920);assert(decodeBenchFixture(e,100,mv));
 s=policy.update(true,mv,150);
 assert(s.critical&&s.band==BOAT_BATTERY_CRITICAL);
 assert(!readForBoatSafety(e,100,mv)); // selected != energized
 std::cout<<"R27 INA231 voltage-only selection contract tests passed (hardware LOCKED)\n";
}
