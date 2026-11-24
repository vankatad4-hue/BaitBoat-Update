#pragma once
// Cross-endpoint portable integration tests include both endpoint headers.
// Their enum/struct wire ABI must be defined ONCE per translation unit, even
// when the Boat and Remote source directories are included together.
#ifndef FLYTEC_RADIO_PAYLOAD_CODEC_SHARED_TYPES_H
#define FLYTEC_RADIO_PAYLOAD_CODEC_SHARED_TYPES_H
#include <stdint.h>

// Portable wire-payload decoder. No Arduino dependencies; all payloads are
// exactly eight bytes and use little-endian numeric fields.
enum RadioPayloadKind : uint8_t {
    RADIO_PAYLOAD_INVALID = 0,
    RADIO_PAYLOAD_HEARTBEAT,
    RADIO_PAYLOAD_ACK,
    RADIO_PAYLOAD_STATUS,
    RADIO_PAYLOAD_BATTERY_SPEED,
    RADIO_PAYLOAD_GNSS_LATITUDE,
    RADIO_PAYLOAD_GNSS_LONGITUDE,
    RADIO_PAYLOAD_NAVIGATION,
    RADIO_PAYLOAD_SONAR_SUMMARY,
    RADIO_PAYLOAD_WEATHER,
    RADIO_PAYLOAD_SAFETY_DIAGNOSTIC // R30F optional authenticated telemetry subtype 8; NEVER a control permission
};

enum RadioAckResult : uint8_t {
    RADIO_ACK_ACCEPTED = 0,
    RADIO_ACK_EXECUTED = 1,
    RADIO_ACK_REJECTED_SAFETY = 2,
    RADIO_ACK_INVALID_COMMAND = 3,
    RADIO_ACK_STALE = 4,
    RADIO_ACK_DUPLICATE = 5,
    RADIO_ACK_BUSY = 6,
    RADIO_ACK_INTERNAL_FAILURE = 7
};

struct RadioDecodedPayload {
    RadioPayloadKind kind;
    uint8_t subtype;
    uint8_t heartbeatFlags;
    uint8_t buildMajor;
    uint8_t ackResult;
    uint8_t commandType;
    uint16_t commandId;
    uint8_t stopState;
    uint8_t interlockFlags;
    uint8_t boatMode;
    uint8_t safetyFlags;
    uint8_t batteryPercent;
    uint8_t gpsFixType;
    uint8_t satellites;
    uint16_t batteryMillivolts;
    uint16_t speedCmPerSecond;
    uint8_t coordinateSample;
    int32_t coordinateE7;
    uint16_t horizontalAccuracyCm;
    uint16_t headingCentidegrees;
    uint8_t navigationStatus;
    uint8_t waypointIndex;
    uint8_t navigationSample;
    int16_t temperatureCentiC;
    uint32_t pressurePa;
    uint16_t depthCm;
    uint16_t sonarRangeCm;
    uint8_t sonarHealth;
    uint8_t sonarAlarmFlags;
    uint8_t sonarSample;
    uint8_t diagnosticReason;
    uint16_t diagnosticFlags;
    uint16_t diagnosticMillivolts;
    uint8_t diagnosticSequence;
};

// Reject unknown subtype, invalid enums/ranges and nonzero reserved bytes/bits.
// On failure, output is cleared and kind remains INVALID.
bool radio_decodeHeartbeatPayload(const uint8_t payload[8], RadioDecodedPayload &out);
bool radio_decodeAckPayload(const uint8_t payload[8], RadioDecodedPayload &out);
bool radio_decodeTelemetryPayload(const uint8_t payload[8], RadioDecodedPayload &out);

#endif // FLYTEC_RADIO_PAYLOAD_CODEC_SHARED_TYPES_H

// Boat-only declarations remain visible when Remote's shared types were included first.
// Portable little-endian payload encoders. On invalid input, zero the output
// and return false. These encode payloads only, not authenticated RadioFrames.
bool radio_encodeHeartbeatPayload(uint8_t flags,uint8_t buildMajor,uint8_t out[8]);
bool radio_encodeAckPayload(uint8_t result,uint8_t commandType,uint16_t commandId,uint8_t stopState,uint8_t interlockFlags,uint8_t out[8]);
bool radio_encodeTelemetryStatusPayload(uint8_t mode,uint8_t stopState,uint8_t safetyFlags,uint8_t batteryPercent,uint8_t gpsFixType,uint8_t satellites,uint8_t out[8]);
bool radio_encodeBatterySpeedPayload(uint16_t millivolts,uint16_t speedCmPerSecond,uint8_t out[8]);
bool radio_encodeLatitudePayload(uint8_t sample,int32_t latitudeE7,uint16_t accuracyCm,uint8_t out[8]);
bool radio_encodeLongitudePayload(uint8_t sample,int32_t longitudeE7,uint8_t out[8]);
bool radio_encodeNavigationPayload(uint16_t headingCentidegrees,uint16_t speedCmPerSecond,uint8_t navigationStatus,uint8_t waypointIndex,uint8_t sample,uint8_t out[8]);
bool radio_encodeSonarSummaryPayload(uint16_t depthCm,uint16_t rangeCm,uint8_t health,uint8_t alarmFlags,uint8_t sample,uint8_t out[8]);

