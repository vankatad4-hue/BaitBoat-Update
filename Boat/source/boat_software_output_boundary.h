#pragma once
#include <stdint.h>

// R30H-30: pure software output boundary, no external safety hardware.
// A valid request is NOT proof that the driver actually de-energized a load.
namespace boat_r30h30 {
static constexpr uint16_t MAX_COMMAND_LEASE_MS = 100U;
static constexpr int16_t MAX_MOTOR_DEMAND = 1000;
static constexpr int8_t MAX_RELEASE_CHANNEL = 3;
inline bool validOutput(int16_t left, int16_t right, int8_t releaseChannel,
                        uint16_t leaseMs) {
 return leaseMs > 0U && leaseMs <= MAX_COMMAND_LEASE_MS &&
        left >= -MAX_MOTOR_DEMAND && left <= MAX_MOTOR_DEMAND &&
        right >= -MAX_MOTOR_DEMAND && right <= MAX_MOTOR_DEMAND &&
        releaseChannel >= -1 && releaseChannel <= MAX_RELEASE_CHANNEL &&
        (releaseChannel == -1 || (left == 0 && right == 0));
}
} // namespace boat_r30h30
