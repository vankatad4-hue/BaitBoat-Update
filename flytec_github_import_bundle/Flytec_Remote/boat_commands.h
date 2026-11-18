
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
    CMD_HOOK_RELEASE
};

// Complete application-level command.
struct BoatCommand
{
    uint32_t sequence;       // Unique command sequence number.

    BoatCommandType type;    // Command type.

    BoatMode mode;           // Requested operating mode.

    int16_t leftMotor;       // Left motor: -1000 to +1000.
    int16_t rightMotor;      // Right motor: -1000 to +1000.

    bool activate;           // Solenoid activation request.
};

// Function pointer used by the radio transport module.
// The radio module will implement the actual transmission.
typedef bool (*BoatCommandTransport)(const BoatCommand &command);

// Register the radio transport function.
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

// Request bait release.
bool boatCommands_releaseBait();

// Request hook release.
bool boatCommands_releaseHook();

#endif
