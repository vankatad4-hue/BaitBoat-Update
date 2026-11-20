
#include "boat_commands.h"

// Next command sequence number.
static uint32_t commandSequence = 0;

// Registered radio transport function.
static BoatCommandTransport commandTransport = nullptr;

// --------------------------------------------------
// REGISTER RADIO TRANSPORT
// --------------------------------------------------

void boatCommands_setTransport(BoatCommandTransport transport)
{
    // Save the radio transmission function.
    commandTransport = transport;
}

// --------------------------------------------------
// INTERNAL COMMAND SENDER
// --------------------------------------------------

static bool sendCommand(BoatCommand &command)
{
    // Do not transmit if no radio transport is registered.
    if (commandTransport == nullptr)
    {
        return false;
    }

    // Assign a unique logical ID. Link retries must retain this ID so the
    // boat can suppress duplicate execution across nRF/LoRa failover.
    command.sequence = ++commandSequence;
    command.commandId = command.sequence;
    command.createdAtMs = millis();

    // Send the command through the registered transport.
    return commandTransport(command);
}

// --------------------------------------------------
// EMERGENCY STOP
// --------------------------------------------------

bool boatCommands_sendStop()
{
    // Create an emergency stop command.
    BoatCommand command = {};

    command.type = CMD_EMERGENCY_STOP;

    // Emergency stop has priority in the boat firmware.
    return sendCommand(command);
}

// --------------------------------------------------
// CLEAR EMERGENCY STOP
// --------------------------------------------------

bool boatCommands_clearStop()
{
    // Create a stop-clear request.
    BoatCommand command = {};

    command.type = CMD_CLEAR_STOP;

    // The boat must decide whether clearing is safe.
    return sendCommand(command);
}

// --------------------------------------------------
// CHANGE OPERATING MODE
// --------------------------------------------------

bool boatCommands_setMode(BoatMode mode)
{
    // Reject invalid mode values.
    if (mode != MODE_MANUAL &&
        mode != MODE_AUTOPILOT &&
        mode != MODE_RETURN_HOME)
    {
        return false;
    }

    // Create the mode-change command.
    BoatCommand command = {};

    command.type = CMD_SET_MODE;
    command.mode = mode;

    // Send the requested mode.
    return sendCommand(command);
}

// --------------------------------------------------
// MANUAL MOTOR CONTROL
// --------------------------------------------------

bool boatCommands_setManualMotors(int16_t left, int16_t right)
{
    // Restrict left motor command to the allowed range.
    left = constrain(left, -1000, 1000);

    // Restrict right motor command to the allowed range.
    right = constrain(right, -1000, 1000);

    // Create a manual motor command.
    BoatCommand command = {};

    command.type = CMD_MANUAL_MOTORS;

    // Store both motor commands.
    command.leftMotor = left;
    command.rightMotor = right;

    // Send the command.
    return sendCommand(command);
}

// --------------------------------------------------
// BAIT RELEASE
// --------------------------------------------------

bool boatCommands_releaseBait()
{
    // Create a bait-release command.
    BoatCommand command = {};

    command.type = CMD_BAIT_RELEASE;
    command.activate = true;

    // Send the request.
    return sendCommand(command);
}

// --------------------------------------------------
// HOOK RELEASE
// --------------------------------------------------

bool boatCommands_releaseHook()
{
    // Create a hook-release command.
    BoatCommand command = {};

    command.type = CMD_HOOK_RELEASE;
    command.activate = true;

    // Send the request.
    return sendCommand(command);
}
