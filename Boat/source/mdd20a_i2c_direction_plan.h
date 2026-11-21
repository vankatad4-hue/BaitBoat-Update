#pragma once
#include <stdint.h>
// R34: pure, non-actuating policy for a future factory-made I2C GPIO module.
// NO I2C writes, motor outputs, GPIO initialization, or actuator authorization.
// Both direction bits MUST be committed in one verified expander transaction;
// independent hardware contactor/STOP is mandatory.
struct Mdd20aI2cDirectionPlan {
  uint8_t bits;
  bool valid;
};
inline Mdd20aI2cDirectionPlan mdd20aI2cDirectionOff() {
  return {0,false};
}
inline Mdd20aI2cDirectionPlan mdd20aI2cDirectionPrepare(
    bool commissioningSigned,bool stopHealthy,bool motorSupplyIsolated,
    bool pwmConfirmedZero,bool busHealthy,bool reverseLeft,bool reverseRight) {
  if(!commissioningSigned||!stopHealthy||!motorSupplyIsolated||
     !pwmConfirmedZero||!busHealthy)return mdd20aI2cDirectionOff();
  return {static_cast<uint8_t>((reverseLeft?1U:0U)|(reverseRight?2U:0U)),true};
}
// An I2C expander ACK is not proof that physical DIR outputs changed.
// Only separately measured feedback may satisfy the verification gate.
inline bool mdd20aI2cDirectionCommitVerified(
    Mdd20aI2cDirectionPlan p,bool i2cAck,bool physicalFeedbackValid,
    uint8_t measuredBits,bool pwmConfirmedZero,bool motorSupplyIsolated) {
  return p.valid&&i2cAck&&physicalFeedbackValid&&
         ((measuredBits&3U)==p.bits)&&pwmConfirmedZero&&motorSupplyIsolated;
}
