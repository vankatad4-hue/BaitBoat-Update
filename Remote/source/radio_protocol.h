#pragma once
#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

// Fixed 32-byte application frame: fits nRF24 maximum payload and is also
// used as the LoRa application payload. All fields are little-endian.
static constexpr uint8_t RADIO_FRAME_MAGIC0 = 0xB6;
static constexpr uint8_t RADIO_FRAME_MAGIC1 = 0x4D;
static constexpr uint8_t RADIO_PROTOCOL_VERSION = 1;
static constexpr size_t RADIO_FRAME_SIZE = 32;
static constexpr size_t RADIO_AUTH_TAG_SIZE = 8;

enum RadioMessageType : uint8_t {
    RADIO_MSG_HEARTBEAT = 1,
    RADIO_MSG_CONTROL = 2,
    RADIO_MSG_TELEMETRY = 3,
    RADIO_MSG_ACTION = 4,
    RADIO_MSG_ACK = 5,
    RADIO_MSG_LINK_PROBE = 6
};

enum RadioAction : uint8_t {
    RADIO_ACTION_NONE = 0,
    RADIO_ACTION_STOP = 1,
    RADIO_ACTION_RETURN_HOME = 2,
    RADIO_ACTION_RELEASE_BAIT = 3,
    RADIO_ACTION_RELEASE_HOOK = 4,
    RADIO_ACTION_MISSION_START = 5
};

#pragma pack(push, 1)
struct RadioFrame {
    uint8_t magic[2];
    uint8_t version;
    uint8_t type;
    uint8_t sourceId;
    uint8_t destinationId;
    uint16_t sequence;
    uint16_t commandId;
    uint16_t tick16;       // milliseconds modulo 65536; freshness uses wrap-safe delta
    uint8_t flags;
    uint8_t payload[8];
    uint16_t crc16;
    uint8_t authTag[8];    // truncated HMAC-SHA256; provision a unique key, never a shared default
    uint8_t reserved;
};
#pragma pack(pop)

static_assert(sizeof(RadioFrame) == RADIO_FRAME_SIZE, "RadioFrame must be 32 bytes");

uint16_t radio_crc16(const uint8_t *data, size_t length);
bool radio_frameSeal(RadioFrame &frame, const uint8_t key[32]);
bool radio_frameVerify(const RadioFrame &frame, const uint8_t key[32]);
void radio_frameInit(RadioFrame &frame, RadioMessageType type, uint8_t src, uint8_t dst,
                     uint16_t seq, uint16_t cmdId, uint32_t nowMs);
bool radio_tickFresh(uint16_t receivedTick, uint16_t nowTick, uint16_t maxAgeMs);
