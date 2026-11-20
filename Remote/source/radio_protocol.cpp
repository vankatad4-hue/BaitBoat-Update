#include "radio_protocol.h"
#include <string.h>
#include <mbedtls/md.h>

// CRC-16/CCITT-FALSE. Error detection is separate from authentication.
uint16_t radio_crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; ++bit)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

static bool hmacTag(const uint8_t key[32], const uint8_t *data, size_t length, uint8_t out[8]) {
    uint8_t digest[32];
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!info || mbedtls_md_hmac(info, key, 32, data, length, digest) != 0) return false;
    memcpy(out, digest, 8);
    memset(digest, 0, sizeof(digest));
    return true;
}

void radio_frameInit(RadioFrame &f, RadioMessageType type, uint8_t src, uint8_t dst,
                     uint16_t seq, uint16_t cmdId, uint32_t nowMs, uint8_t sessionEpoch) {
    memset(&f, 0, sizeof(f));
    f.magic[0] = RADIO_FRAME_MAGIC0;
    f.magic[1] = RADIO_FRAME_MAGIC1;
    f.version = RADIO_PROTOCOL_VERSION;
    f.type = (uint8_t)type;
    f.sourceId = src;
    f.destinationId = dst;
    f.sequence = seq;
    f.commandId = cmdId;
    f.tick16 = (uint16_t)nowMs;
    f.sessionEpoch = sessionEpoch;
}

bool radio_frameSeal(RadioFrame &f, const uint8_t key[32]) {
    f.crc16 = radio_crc16((const uint8_t*)&f, offsetof(RadioFrame, crc16));
    memset(f.authTag, 0, sizeof(f.authTag));
    return hmacTag(key, (const uint8_t*)&f, offsetof(RadioFrame, authTag), f.authTag);
}

bool radio_frameVerify(const RadioFrame &f, const uint8_t key[32]) {
    if (f.magic[0] != RADIO_FRAME_MAGIC0 || f.magic[1] != RADIO_FRAME_MAGIC1 ||
        f.version != RADIO_PROTOCOL_VERSION) return false;
    if (radio_crc16((const uint8_t*)&f, offsetof(RadioFrame, crc16)) != f.crc16) return false;
    uint8_t expected[8];
    if (!hmacTag(key, (const uint8_t*)&f, offsetof(RadioFrame, authTag), expected)) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < 8; ++i) diff |= expected[i] ^ f.authTag[i];
    memset(expected, 0, sizeof(expected));
    return diff == 0;
}

bool radio_tickFresh(uint16_t receivedTick, uint16_t nowTick, uint16_t maxAgeMs) {
    const uint16_t age = (uint16_t)(nowTick - receivedTick);
    return age <= maxAgeMs;
}
