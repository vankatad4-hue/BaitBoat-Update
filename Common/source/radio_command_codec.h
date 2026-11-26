#pragma once
#include "radio_protocol.h"
#include "boat_commands.h"

// Encode a command into an authenticated frame. Callers must supply a
// provisioned 32-byte secret key and a non-zero session epoch.
bool radio_encodeCommand(const BoatCommand &cmd, uint8_t sourceId, uint8_t destinationId,
                         uint8_t sessionEpoch, RadioFrame &frame, const uint8_t key[32]);

// Decode and authenticate a command frame. Backup-link policy is enforced here
// as a defense-in-depth check; boat actuator logic must validate again.
bool radio_decodeCommand(const RadioFrame &frame, uint8_t expectedSourceId,
                         uint8_t expectedDestinationId, uint32_t nowMs,
                         uint16_t maxAgeMs, bool backupLink, BoatCommand &out,
                         const uint8_t key[32]);

class RadioSequenceGuard {
public:
    RadioSequenceGuard() : initialized_(false), epoch_(0), lastSequence_(0) {}
    void reset() { initialized_=false; epoch_=0; lastSequence_=0; }
    // Return true only for a new forward sequence; epoch changes are denied unless authorized.
    bool accept(uint8_t epoch, uint16_t sequence, bool allowEpochReset=false);
private:
    bool initialized_;
    uint8_t epoch_;
    uint16_t lastSequence_;
};
