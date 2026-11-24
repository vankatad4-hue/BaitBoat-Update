
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>
#include "boat_state.h"

// --------------------------------------------------
// CONTROL SETTINGS
// --------------------------------------------------

enum ControlLayout : uint8_t
{
    CONTROL_TANK = 0,
    CONTROL_STEERING = 1
};

enum AutopilotOverride : uint8_t
{
    OVERRIDE_TEMPORARY = 0,
    OVERRIDE_TO_MANUAL = 1
};

// --------------------------------------------------
// RAW JOYSTICK READINGS
// --------------------------------------------------

struct JoystickRaw
{
    int leftX;
    int leftY;
    int rightX;
    int rightY;
};

// --------------------------------------------------
// NORMALISED JOYSTICK READINGS
// --------------------------------------------------

struct JoystickValues
{
    int16_t leftX;
    int16_t leftY;
    int16_t rightX;
    int16_t rightY;
};

// --------------------------------------------------
// CALIBRATION DATA FOR ONE AXIS
// --------------------------------------------------

struct AxisCalibration
{
    int minimum;
    int centre;
    int maximum;
};

// Calibration data for all four axes.
struct JoystickCalibration
{
    AxisCalibration leftX;
    AxisCalibration leftY;
    AxisCalibration rightX;
    AxisCalibration rightY;

    int deadZone;
};

// --------------------------------------------------
// DATA SHARED WITH THE DISPLAY
// --------------------------------------------------

enum CentreCaptureState : uint8_t { CENTRE_IDLE, CENTRE_RUNNING, CENTRE_COMPLETE, CENTRE_FAILED };

struct JoystickSnapshot
{
    JoystickRaw raw;
    JoystickValues values;

    JoystickCalibration calibration;

    bool readingsValid;
    bool calibrationValid;
    bool neutral;
    bool calibrationMode;
    bool endpointCapture;
    CentreCaptureState centreCaptureState;
    uint8_t centreSamples;
};

// --------------------------------------------------
// HARDWARE READING CALLBACK
// --------------------------------------------------

typedef bool (*JoystickReadCallback)(JoystickRaw &raw);

// --------------------------------------------------
// INITIALISATION AND HARDWARE
// --------------------------------------------------

void joystick_begin();

void joystick_setReadCallback(JoystickReadCallback callback);

// --------------------------------------------------
// CONTROL SETTINGS
// --------------------------------------------------

void joystick_setLayout(ControlLayout layout);

ControlLayout joystick_getLayout();

void joystick_setAutopilotOverride(AutopilotOverride behaviour);

AutopilotOverride joystick_getAutopilotOverride();

// --------------------------------------------------
// CALIBRATION
// --------------------------------------------------

// Enter calibration mode only after confirmed emergency STOP.
bool joystick_calibrationEnter();

// Leave calibration mode.
bool joystick_calibrationExit();

// Request cooperative centre capture; true means started, not completed.
bool joystick_calibrateCentre();

// Start recording the minimum and maximum endpoint positions.
bool joystick_beginEndpointCapture();

// Finish endpoint recording and validate the results.
bool joystick_endEndpointCapture();

// Change the dead zone in raw ADC counts.
void joystick_setDeadZone(int deadZone);

// Read current calibration settings and joystick readings.
JoystickSnapshot joystick_getSnapshot();

// --------------------------------------------------
// NORMAL OPERATION
// --------------------------------------------------

// Process joystick readings and, when safe, send motor commands.
void joystick_update();

// Check whether the joysticks are neutral.
bool joystick_isNeutral();

// Read the latest normalised values.
JoystickValues joystick_getValues();

#endif
