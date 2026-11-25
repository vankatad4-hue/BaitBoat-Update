#pragma once
#include <stdint.h>

// R16 OWNER-SELECTED module: Electrokit EKM014 41024394.
// PURELY LOGICAL mapping: DOES NOT define ESP32 pins, switch a MOSFET,
// implement one-shot timing or authorize actuator power. Hardware E-STOP,
// independent no-retrigger timer, current-zero and POR remain unselected.
namespace flytec_ekm014_r16 {
enum Channel : int8_t {
    LH_BAIT = 0, RH_BAIT = 1, LK_HOOK = 2, RK_HOOK = 3,
    NONE = -1
};
inline const char* inputName(Channel channel) {
    switch(channel) {
        case LH_BAIT: return "IN1";
        case RH_BAIT: return "IN2";
        case LK_HOOK: return "IN3";
        case RK_HOOK: return "IN4";
        default: return "NONE";
    }
}
inline const char* outputName(Channel channel) {
    switch(channel) {
        case LH_BAIT: return "CH1";
        case RH_BAIT: return "CH2";
        case LK_HOOK: return "CH3";
        case RK_HOOK: return "CH4";
        default: return "NONE";
    }
}
inline uint8_t logicalMask(Channel channel) {
    if(channel < LH_BAIT || channel > RK_HOOK) return 0;
    return static_cast<uint8_t>(1u << static_cast<uint8_t>(channel));
}
inline bool isSingleChannelMask(uint8_t mask) {
    return mask == 0x01 || mask == 0x02 || mask == 0x04 || mask == 0x08;
}
inline bool proposedSupplyWithinManufacturerBounds(float vh, float vl) {
    // 5-50V VH, 4.5-12V VL, and VH>=VL. Floating NaN fails automatically.
    // A true result proves only mathematical envelope, NOT physical safety.
    return vh >= 5.0f && vh <= 50.0f &&
           vl >= 4.5f && vl <= 12.0f && vh >= vl;
}
}
