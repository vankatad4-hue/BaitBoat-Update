#pragma once
#include <stdint.h>
#include "radio_payload_codec.h"

enum RadioAckLink : uint8_t { RADIO_ACK_LINK_NONE=0, RADIO_ACK_LINK_PRIMARY=1, RADIO_ACK_LINK_BACKUP=2 };
enum RadioCommandAckState : uint8_t {
    RADIO_CMD_ACK_UNKNOWN=0, RADIO_CMD_ACK_WAITING, RADIO_CMD_ACK_ACCEPTED,
    RADIO_CMD_ACK_EXECUTED, RADIO_CMD_ACK_DUPLICATE, RADIO_CMD_ACK_REJECTED,
    RADIO_CMD_ACK_TIMEOUT
};
struct RadioCommandAckRecord {
    uint16_t commandId;
    uint8_t commandType;
    uint8_t requestedMode;
    RadioCommandAckState state;
    uint8_t lastResult;
    uint8_t stopState;
    uint8_t interlockFlags;
    uint32_t createdAtMs;
    uint32_t updatedAtMs;
};
class RadioAckTracker {
public:
    static constexpr uint8_t CAPACITY=8;
    RadioAckTracker();
    void reset(uint8_t sessionEpoch);
    bool begin(uint16_t commandId,uint8_t commandType,uint8_t requestedMode,uint32_t nowMs,uint16_t timeoutMs=2000);
    bool observe(const RadioDecodedPayload &ack,RadioAckLink link,uint8_t sessionEpoch,uint32_t nowMs);
    void update(uint32_t nowMs);
    bool get(uint16_t commandId,RadioCommandAckRecord &out) const;
    uint8_t pendingCount() const;
private:
    struct Slot { RadioCommandAckRecord record; uint16_t timeoutMs; bool used; };
    Slot slots_[CAPACITY];
    uint8_t epoch_;
    bool primaryOnly(uint8_t type,uint8_t mode) const;
    int find(uint16_t id) const;
    int allocate() const;
};
