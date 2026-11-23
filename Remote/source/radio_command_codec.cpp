#include "radio_command_codec.h"

static int16_t clampMotor(int16_t v) {
    if (v > 1000) return 1000;
    if (v < -1000) return -1000;
    return v;
}
static bool backupAllowed(const BoatCommand &cmd) {
    if (cmd.type == CMD_EMERGENCY_STOP) return true;
    if (cmd.type == CMD_SET_MODE) return cmd.mode == MODE_RETURN_HOME || cmd.mode == MODE_AUTOPILOT;
    if (cmd.type == CMD_MANUAL_MOTORS || cmd.type == CMD_CLEAR_STOP) return false;
    // Explicit one-channel release commands still pass Boat interlocks.
    return cmd.type == CMD_RELEASE_HOPPER_LEFT || cmd.type == CMD_RELEASE_HOPPER_RIGHT ||
           cmd.type == CMD_RELEASE_HOOK_LEFT || cmd.type == CMD_RELEASE_HOOK_RIGHT;
}

bool radio_encodeCommand(const BoatCommand &cmd, uint8_t src, uint8_t dst,
                        uint8_t epoch, RadioFrame &f, const uint8_t key[32]) {
    const RadioMessageType type = (cmd.type == CMD_MANUAL_MOTORS) ? RADIO_MSG_CONTROL : RADIO_MSG_ACTION;
    radio_frameInit(f, type, src, dst, (uint16_t)cmd.sequence, (uint16_t)cmd.commandId,
                    cmd.createdAtMs, epoch);
    f.payload[0] = (uint8_t)cmd.type;
    f.payload[1] = (uint8_t)cmd.mode;
    const int16_t left=clampMotor(cmd.leftMotor), right=clampMotor(cmd.rightMotor);
    f.payload[2]=(uint8_t)(left & 0xFF); f.payload[3]=(uint8_t)((uint16_t)left >> 8);
    f.payload[4]=(uint8_t)(right & 0xFF); f.payload[5]=(uint8_t)((uint16_t)right >> 8);
    f.payload[6]=cmd.activate ? 1 : 0;
    f.payload[7]=0;
    return radio_frameSeal(f,key);
}

bool radio_decodeCommand(const RadioFrame &f, uint8_t expectedSrc, uint8_t expectedDst,
                        uint32_t nowMs, uint16_t maxAgeMs, bool backup,
                        BoatCommand &out, const uint8_t key[32]) {
    if (!radio_frameVerify(f,key)) return false;
    if (f.sourceId != expectedSrc || f.destinationId != expectedDst) return false;
    // Sender and receiver millis() clocks are not synchronized; timestamp age
    // cannot be compared across devices. Freshness is enforced by sequence,
    // local receive watchdog, and command queue age. Keep parameter for API.
    (void)maxAgeMs;
    if (f.type != RADIO_MSG_CONTROL && f.type != RADIO_MSG_ACTION) return false;
    if (f.payload[7] != 0 || f.payload[6] > 1) return false;
    const uint8_t rawType=f.payload[0];
    if (rawType < CMD_EMERGENCY_STOP || rawType > CMD_RELEASE_HOOK_RIGHT) return false;
    out = {};
    out.sequence=f.sequence;
    out.commandId=f.commandId;
    out.createdAtMs=nowMs;
    out.type=(BoatCommandType)rawType;
    out.mode=(BoatMode)f.payload[1];
    out.leftMotor=(int16_t)((uint16_t)f.payload[2] | ((uint16_t)f.payload[3]<<8));
    out.rightMotor=(int16_t)((uint16_t)f.payload[4] | ((uint16_t)f.payload[5]<<8));
    out.activate=f.payload[6]!=0;
    out.leftMotor=clampMotor(out.leftMotor); out.rightMotor=clampMotor(out.rightMotor);
    if (out.mode != MODE_MANUAL && out.mode != MODE_AUTOPILOT && out.mode != MODE_RETURN_HOME)
        return false;
    const bool releaseCommand = (out.type == CMD_RELEASE_HOPPER_LEFT || out.type == CMD_RELEASE_HOPPER_RIGHT ||
        out.type == CMD_RELEASE_HOOK_LEFT || out.type == CMD_RELEASE_HOOK_RIGHT);
    // Legacy combined-release IDs are rejected to prevent unintended two-channel actuation.
    if (out.type == CMD_BAIT_RELEASE || out.type == CMD_HOOK_RELEASE) return false;
    if (out.activate && !releaseCommand) return false;
    if (releaseCommand && !out.activate) return false;
    if (backup && !backupAllowed(out)) return false;
    return true;
}

bool RadioSequenceGuard::accept(uint8_t epoch, uint16_t sequence, bool allowEpochReset) {
    if (!initialized_) { initialized_=true; epoch_=epoch; lastSequence_=sequence; return true; }
    if (epoch != epoch_) {
        if (!allowEpochReset) return false;
        epoch_=epoch;lastSequence_=sequence;return true;
    }
    const uint16_t delta=(uint16_t)(sequence-lastSequence_);
    if (delta==0 || delta>=0x8000) return false; // duplicate, backwards or stale
    lastSequence_=sequence;
    return true;
}
