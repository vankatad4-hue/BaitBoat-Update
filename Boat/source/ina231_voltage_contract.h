#pragma once
#include <stdint.h>
// R27 hardware selection: Ardboard INA231 product 1491 (voltage-only).
// Selected for procurement; NOT purchased or commissioned. This pure host
// contract neither calls Wire nor permits the Boat physical-driver adapter
// to start. Existing legacy ADS1115 sensor code remains inactive/unsigned.
namespace boat_r27 {
static constexpr bool INA231_MODULE_SELECTED=true;
static constexpr bool INA231_PHYSICAL_COMMISSIONED=false;
static constexpr uint8_t I2C_CANDIDATE_ADDRESS=0x40; // A1/A0 both GND, inspect board
static constexpr uint8_t REG_CONFIG=0x00, REG_BUS=0x02, REG_MASK=0x06;
// TI INA231 Rev D: 0x4126 = 1 sample, VBUS CT 1.1ms,
// VSH CT 1.1ms (unused), MODE=110 voltage-only continuous.
static constexpr uint16_t CONFIG_VBUS_CONTINUOUS=0x4126;
static constexpr uint32_t MAX_SAMPLE_AGE_MS=1000U;
struct VoltageOnlyEvidence {
 bool pcbAndBusContinuityVerified=false; // BUS separate from R100 IN+/IN-
 bool shuntNotInPowerPath=false; // 20A main feed never goes through R100
 bool i2cDomainVerified=false; // module 3.3V, address, startup order
 bool dmmCalibrationSigned=false;
 bool newConversionConfirmed=false; // mandatory: not a cached I2C register
 bool readAndConfigVerified=false;
 bool applicationProfileCommissioned=false;
 uint8_t actualAddress=0;
 uint16_t configurationReadback=0;
 uint16_t rawBusVoltage=0;
 uint32_t conversionAtMs=0;
};
inline uint32_t rawToMillivolts(uint16_t raw) {
 return (static_cast<uint32_t>(raw)*5U+2U)/4U;
}
inline bool decodeBenchFixture(const VoltageOnlyEvidence& e,uint32_t now,
                               uint16_t& millivolts) {
 millivolts=0;
 if(!e.pcbAndBusContinuityVerified || !e.shuntNotInPowerPath ||
    !e.i2cDomainVerified || !e.dmmCalibrationSigned ||
    !e.newConversionConfirmed || !e.readAndConfigVerified ||
    !e.applicationProfileCommissioned ||
    e.actualAddress!=I2C_CANDIDATE_ADDRESS ||
    e.configurationReadback!=CONFIG_VBUS_CONTINUOUS ||
    (e.rawBusVoltage&0x8000U)!=0U ||
    static_cast<uint32_t>(now-e.conversionAtMs)>MAX_SAMPLE_AGE_MS) return false;
 const uint32_t mv=rawToMillivolts(e.rawBusVoltage);
 if(mv<9000U||mv>13000U)return false; // existing 3S plausibility gate
 millivolts=static_cast<uint16_t>(mv);return true;
}
inline bool readForBoatSafety(const VoltageOnlyEvidence&,uint32_t,
                               uint16_t& millivolts) {
 // Physical gate is deliberately compile-time FALSE until measured/purchased.
 millivolts=0;return false;
}
} // namespace boat_r27
