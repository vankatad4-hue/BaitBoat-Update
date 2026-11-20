#pragma once
#include "radio_payload_codec.h"

// Portable wire-payload encoder. Returns false for invalid fields and clears
// output on failure. This is protocol utility code only, not actuator logic.
bool radio_encodeHeartbeatPayload(uint8_t flags, uint8_t buildMajor, uint8_t out[8]);
bool radio_encodeAckPayload(uint8_t result, uint8_t commandType, uint16_t commandId,
                            uint8_t stopState, uint8_t interlockFlags, uint8_t out[8]);
bool radio_encodeTelemetryPayload(const RadioDecodedPayload &in, uint8_t out[8]);
