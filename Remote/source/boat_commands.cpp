
#include "boat_commands.h"
#include "boat_radio.h"
#include "joystick.h"

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
    const auto j=joystick_getSnapshot();
    if(!boatState.emergencyStop||!boatState.emergencyStopConfirmed||!boatRadio_primaryConnected()||boatState.motorLeftActive||boatState.motorRightActive||!j.readingsValid||!j.calibrationValid||j.calibrationMode||!j.neutral)return false;
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

    // Home is an irreversible safety diversion for a previously started
    // mission only. Boat validates its locally stored Home, GPS and STOP.
    if(mode==MODE_RETURN_HOME){
        const auto phase=boatRadio_missionPhase();
        if((phase!=MC_OUTBOUND && phase!=MC_PAUSED &&
            phase!=MC_RETURNING && phase!=MC_OVERRIDE_NEUTRAL &&
            phase!=MC_OVERRIDE_ACTIVE) ||
           !boatState.emergencyStopConfirmed || boatState.emergencyStop ||
           boatState.failsafeActive)return false;
        BoatCommand home={};home.type=CMD_SET_MODE;home.mode=MODE_RETURN_HOME;
        return sendCommand(home);
    }
    // Other autonomous mode changes use the mission protocol.
    if(boatRadio_missionLocked())return mode==MODE_MANUAL?boatRadio_missionControl(MCTRL_SWITCH_MANUAL):mode==MODE_AUTOPILOT?boatRadio_missionControl(MCTRL_START):false;
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
    if(!boatRadio_manualModeConfirmed() || boatState.mode!=MODE_MANUAL || !boatState.radioConnected ||
       boatState.radioActiveLink!=RADIO_UI_NRF_PRIMARY ||
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

bool boatCommands_setSonarEnabled(bool enabled, uint16_t *commandId)
{
    if (!commandId || !boatRadio_primaryConnected() || (enabled && boatState.failsafeActive))
        return false;
    BoatCommand command = {};
    command.type = CMD_SONAR_POWER;
    command.mode = MODE_MANUAL;
    command.activate = enabled;
    if (!sendCommand(command)) return false;
    *commandId = static_cast<uint16_t>(command.commandId);
    return *commandId != 0U;
}

// Release actions fail closed until authenticated primary-link telemetry
// confirms STOP released and the boat-side actuator interlocks are healthy.
static bool releaseRequestAllowed()
{
    return boatRadio_manualModeConfirmed() && boatState.mode == MODE_MANUAL && !boatState.failsafeActive &&
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
