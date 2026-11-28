#ifndef BOAT_COMMANDS_H
#define BOAT_COMMANDS_H
#include "boat_state.h"
#include <Arduino.h>

// Commands that the remote can send to the boat.
enum BoatCommandType : uint8_t
{
    CMD_EMERGENCY_STOP = 1,
    CMD_CLEAR_STOP,
    CMD_SET_MODE,
    CMD_MANUAL_MOTORS,
    CMD_BAIT_RELEASE,
    CMD_HOOK_RELEASE,
    CMD_RELEASE_HOPPER_LEFT = 7,
    CMD_RELEASE_HOPPER_RIGHT = 8,
    CMD_RELEASE_HOOK_LEFT = 9,
    CMD_RELEASE_HOOK_RIGHT = 10,
    CMD_SONAR_POWER = 11
};

// Complete application-level command.
// commandId remains the logical identity across transport retries/failover,
// allowing the boat to deduplicate actions. createdAtMs is local send time.
struct BoatCommand
{
    uint32_t sequence;        // Monotonic packet sequence.
    uint32_t commandId;       // Stable action ID; assigned with sequence.
    uint32_t createdAtMs;     // Timestamp for stale-command rejection.

    BoatCommandType type;     // Command type.
    BoatMode mode;            // Requested operating mode.

    int16_t leftMotor;        // Left motor: -1000 to +1000.
    int16_t rightMotor;       // Right motor: -1000 to +1000.

    bool activate;            // Solenoid activation request.
};

// Function pointer used by LinkManager/radio transport.
typedef bool (*BoatCommandTransport)(const BoatCommand &command);

// Register the radio/link supervisor transport function.
void boatCommands_setTransport(BoatCommandTransport transport);

// Send an emergency stop command.
bool boatCommands_sendStop();

// Request release of the emergency stop.
// The boat must independently validate this request.
bool boatCommands_clearStop();

// Request a new operating mode.
bool boatCommands_setMode(BoatMode mode);

// Send manual motor commands.
bool boatCommands_setManualMotors(int16_t left, int16_t right);

// Request a Sonar ESP32 state change. `commandId` receives the ID used to
// correlate the eventual authenticated Sonar-state acknowledgement.
bool boatCommands_setSonarEnabled(bool enabled, uint16_t *commandId);

// Request bait release.
bool boatCommands_releaseBait();

// Request hook release.
bool boatCommands_releaseHook();

// Independent fail-closed release commands for each mechanism.
bool boatCommands_releaseHopperLeft();
bool boatCommands_releaseHopperRight();
bool boatCommands_releaseHookLeft();
bool boatCommands_releaseHookRight();

#endif
