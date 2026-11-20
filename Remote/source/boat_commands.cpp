
#include "boat_commands.h"

// Next command sequence number.
static uint32_t commandSequence = 0;
static uint16_t wireCommandIdSequence = 0;

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
    wireCommandIdSequence++;
    if (wireCommandIdSequence == 0) wireCommandIdSequence++;
    command.commandId = wireCommandIdSequence;
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

    // No autonomous mode request until authenticated mission upload/feedback is wired.
    if (mode != MODE_MANUAL) return false;
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
    // Defense in depth: reject movement requests outside confirmed manual operation.
    if(boatState.mode!=MODE_MANUAL || !boatState.radioConnected ||
       !boatState.emergencyStopConfirmed || boatState.emergencyStop ||
       boatState.failsafeActive || !boatState.actuatorSafetyReady)return false;

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

// Release actions fail closed until authenticated primary-link telemetry
// confirms STOP released and the boat-side actuator interlocks are healthy.
static bool releaseRequestAllowed()
{
    return boatState.mode == MODE_MANUAL && !boatState.failsafeActive &&
           boatState.radioConnected &&
           boatState.radioActiveLink == RADIO_UI_NRF_PRIMARY &&
           boatState.emergencyStopConfirmed && !boatState.emergencyStop &&
           boatState.actuatorSafetyReady;
}

// --------------------------------------------------
// Combined hopper/hook commands are intentionally disabled; the new
// independent channel functions below are the only release API.
bool boatCommands_releaseBait(){ return false; }
bool boatCommands_releaseHook(){ return false; }

static bool sendIndependentRelease(BoatCommandType type)
{
    if (!releaseRequestAllowed()) return false;
    BoatCommand command = {};
    command.type = type;
    command.activate = true;
    return sendCommand(command);
}

bool boatCommands_releaseHopperLeft(){ return sendIndependentRelease(CMD_RELEASE_HOPPER_LEFT); }
bool boatCommands_releaseHopperRight(){ return sendIndependentRelease(CMD_RELEASE_HOPPER_RIGHT); }
bool boatCommands_releaseHookLeft(){ return sendIndependentRelease(CMD_RELEASE_HOOK_LEFT); }
bool boatCommands_releaseHookRight(){ return sendIndependentRelease(CMD_RELEASE_HOOK_RIGHT); }
