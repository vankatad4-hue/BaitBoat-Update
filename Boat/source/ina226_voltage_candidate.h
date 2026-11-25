#pragma once
#include <stdint.h>

// R26: HOST-ONLY candidate voltage decoder for TI INA226 (not selected).
// No Wire/I2C GPIO access, no sensor or actuator initialization, no output
// authorization. The existing ADS1115 R21 hardware remains the active choice.
namespace boat_r26 {
static constexpr bool INA226_HARDWARE_SELECTED = false;
static constexpr uint8_t INA226_CANDIDATE_ADDR = 0x40; // actual A0/A1 straps require audit
static constexpr uint8_t REG_BUS_VOLTAGE = 0x02;
static constexpr uint8_t REG_MANUFACTURER = 0xFE;
static constexpr uint8_t REG_DIE_ID = 0xFF;
static constexpr uint16_t TI_MANUFACTURER_ID = 0x5449;
static constexpr uint32_t MAX_AGE_MS = 1000;

struct VoltageEvidence {
 bool moduleRevisionSigned = false;
 bool busPinoutAndAddressSigned = false;
 bool i2cReadOk = false;
 bool newConversionObserved = false; // must not re-timestamp a stale register
 bool dmmCalibrationSigned = false;
 bool profileCommissioned = false;
 uint8_t address = 0;
 uint16_t manufacturerId = 0;
 uint16_t dieId = 0;
 uint16_t rawBusRegister = 0; // register 02h, NOT ADS1115 0.125mV codes
 uint32_t acquiredAtMs = 0;
};

inline uint16_t fromBigEndian(uint8_t msb, uint8_t lsb) {
 return static_cast<uint16_t>((static_cast<uint16_t>(msb) << 8) | lsb);
}
inline bool idMatches(uint16_t manufacturer, uint16_t die) {
 return manufacturer == TI_MANUFACTURER_ID && (die == 0x2260 || die == 0x2261);
}
inline bool decodeEvidenceOnly(const VoltageEvidence &e, uint32_t nowMs,
                           uint16_t &packMillivolts) {
 packMillivolts = 0;
 if (!e.moduleRevisionSigned ||
     !e.busPinoutAndAddressSigned || !e.i2cReadOk ||
     !e.newConversionObserved || !e.dmmCalibrationSigned ||
     !e.profileCommissioned || e.address != INA226_CANDIDATE_ADDR ||
     !idMatches(e.manufacturerId, e.dieId) ||
     static_cast<uint32_t>(nowMs - e.acquiredAtMs) > MAX_AGE_MS ||
     (e.rawBusRegister & 0x8000U) != 0U) return false;
 // INA226 BUS voltage register fixed 1.25mV/LSB (5/4 mV).
 // Round to nearest mV, then enforce the current 3S policy plausibility gate.
 const uint32_t mv = (static_cast<uint32_t>(e.rawBusRegister) * 5U + 2U) / 4U;
 if (mv < 9000U || mv > 13000U) return false;
 packMillivolts = static_cast<uint16_t>(mv);
 return true;
}
// Production-locked entry point: cannot authorize alternate hardware.
inline bool readOnlyDecode(const VoltageEvidence &e, uint32_t nowMs,
                           uint16_t &packMillivolts) {
 if (!INA226_HARDWARE_SELECTED) {packMillivolts=0;return false;}
 return decodeEvidenceOnly(e,nowMs,packMillivolts);
}
// Pure conversion for math/host test fixtures only; NEVER an authorization.
inline uint32_t candidateRegisterMillivolts(uint16_t raw) {
 return (static_cast<uint32_t>(raw) * 5U + 2U) / 4U;
}
} // namespace boat_r26
