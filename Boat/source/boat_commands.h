#pragma once
#include <Arduino.h>
enum BoatMode : uint8_t { MODE_MANUAL=0, MODE_AUTOPILOT=1, MODE_RETURN_HOME=2 };
enum BoatCommandType : uint8_t {
    CMD_EMERGENCY_STOP=1, CMD_CLEAR_STOP, CMD_SET_MODE, CMD_MANUAL_MOTORS,
    CMD_BAIT_RELEASE, CMD_HOOK_RELEASE,
    CMD_RELEASE_HOPPER_LEFT=7, CMD_RELEASE_HOPPER_RIGHT=8,
    CMD_RELEASE_HOOK_LEFT=9, CMD_RELEASE_HOOK_RIGHT=10,
    CMD_SONAR_POWER=11
};
struct BoatCommand {
    uint32_t sequence;
    uint32_t commandId;
    uint32_t createdAtMs;
    BoatCommandType type;
    BoatMode mode;
    int16_t leftMotor;
    int16_t rightMotor;
    bool activate;
};
