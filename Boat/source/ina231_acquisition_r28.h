#pragma once
#include <stdint.h>
#include "ina231_voltage_contract_r27.h"
// R28 transport-neutral, logic-only INA231 voltage sampling contract.
// The normal Boat target MUST pass INA231_PHYSICAL_COMMISSIONED (currently false)
// AND the signed local profile before this sampler can start.
namespace boat_r28 {
class RegisterBus {
public:
 virtual ~RegisterBus() {}
 virtual bool write16(uint8_t addr, uint8_t reg, uint16_t value) = 0;
 virtual bool read16(uint8_t addr, uint8_t reg, uint16_t &value) = 0;
};
// R28 supersedes R27 continuous-mode proposal. A fresh single-shot BUS
// conversion is intentionally triggered for every sample, using CVRF at 06h.
static constexpr uint16_t CONFIG_BUS_TRIGGERED = 0x4122U; // mode 010
static constexpr uint16_t MASK_CONVERSION_READY = 0x0008U; // CVRF bit 3
static constexpr uint16_t MASK_MATH_OVERFLOW = 0x0004U; // OVF bit 2
static constexpr uint32_t MIN_CONVERSION_MS = 2U;
static constexpr uint32_t CONVERSION_TIMEOUT_MS = 40U;
static constexpr uint32_t SAMPLE_PERIOD_MS = 200U;
static constexpr uint32_t MAX_AGE_MS = 1000U;

enum class PollResult : uint8_t { Disabled, Waiting, Fresh, Failed };
struct VoltageSample {
 bool valid;
 uint16_t millivolts;
 uint32_t acquiredAtMs;
};
class VoltageSampler {
public:
 bool begin(bool hardwareAndWiringEvidenceSigned, uint8_t verifiedAddress) {
  reset();
  enabled_ = hardwareAndWiringEvidenceSigned &&
             verifiedAddress == boat_r27::I2C_CANDIDATE_ADDRESS;
  address_ = verifiedAddress;
  return enabled_;
 }
 void reset() {
  enabled_ = false;
  pending_ = false;
  valid_ = false;
  millivolts_ = 0;
  lastAt_ = 0;
  lastStart_ = 0;
  requestAt_ = 0;
  everStarted_ = false;
  address_ = 0;
 }
 VoltageSample snapshot(uint32_t now) const {
  const bool fresh = enabled_ && valid_ &&
                     static_cast<uint32_t>(now-lastAt_) <= MAX_AGE_MS;
  return {fresh, fresh ? millivolts_ : static_cast<uint16_t>(0), lastAt_};
 }
 PollResult poll(RegisterBus &bus, uint32_t now) {
  if (!enabled_) return PollResult::Disabled;
  if (pending_) {
   const uint32_t dt=static_cast<uint32_t>(now-requestAt_);
   if (dt>CONVERSION_TIMEOUT_MS) return fail();
   if (dt<MIN_CONVERSION_MS) return PollResult::Waiting;
   uint16_t mask=0;
   if(!bus.read16(address_, boat_r27::REG_MASK, mask))return fail();
   if(mask&MASK_MATH_OVERFLOW)return fail();
   if(!(mask&MASK_CONVERSION_READY))return PollResult::Waiting;
   uint16_t raw=0;
   if(!bus.read16(address_, boat_r27::REG_BUS, raw))return fail();
   if(raw&0x8000U)return fail();
   const uint32_t mv=boat_r27::rawToMillivolts(raw);
   if(mv<9000U||mv>13000U)return fail();
   pending_=false;
   valid_=true;
   lastAt_=now;
   millivolts_=static_cast<uint16_t>(mv);
   return PollResult::Fresh;
  }
  // A single-shot write is the evidence of a NEW conversion request;
  // repeatedly reading BUS register cannot refresh the sample timestamp.
  if(!everStarted_ || static_cast<uint32_t>(now-lastStart_)>=SAMPLE_PERIOD_MS){
   valid_=snapshot(now).valid; // stale prior value cannot be resurrected
   if(!bus.write16(address_,boat_r27::REG_CONFIG,CONFIG_BUS_TRIGGERED))return fail();
   uint16_t readback=0;
   if(!bus.read16(address_,boat_r27::REG_CONFIG,readback) ||
      readback!=CONFIG_BUS_TRIGGERED)return fail();
   requestAt_=lastStart_=now;
   everStarted_=pending_=true;
  }
  return PollResult::Waiting;
 }
private:
 PollResult fail(){pending_=false;everStarted_=false;valid_=false;millivolts_=0;return PollResult::Failed;}
 bool enabled_=false, pending_=false,valid_=false,everStarted_=false;
 uint8_t address_=0;
 uint16_t millivolts_=0;
 uint32_t lastAt_=0,lastStart_=0,requestAt_=0;
};
} // namespace boat_r28
