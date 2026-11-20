
#include "joystick.h"

#include "boat_commands.h"
#include "boat_radio.h"
#include <Preferences.h>

// --------------------------------------------------
// ADC CONFIGURATION
// --------------------------------------------------

static const int ADC_MINIMUM = 0;
static const int ADC_MAXIMUM = 4095;

// Minimum useful span on either side of the centre.
static const int MINIMUM_AXIS_SPAN = 150;

// Motor commands are limited to approximately 20 Hz.
static const uint32_t COMMAND_INTERVAL_MS = 50;

// --------------------------------------------------
// MODULE SETTINGS
// --------------------------------------------------

static JoystickReadCallback readCallback = nullptr;

static ControlLayout selectedLayout = CONTROL_TANK;

static AutopilotOverride selectedOverride =
    OVERRIDE_TEMPORARY;

// Default calibration for a 12-bit ADC.
// These values MUST be replaced by real calibration.
static JoystickCalibration calibration =
{
    {0, 2048, 4095},
    {0, 2048, 4095},
    {0, 2048, 4095},
    {0, 2048, 4095},
    65
};

// Latest joystick readings.
static JoystickSnapshot snapshot = {};

// Calibration state.
static bool calibrationMode = false;
static bool endpointCapture = false;
static bool calibrationValid = false; // No motor output until a real calibration succeeds.

// Values collected during endpoint calibration.
static JoystickRaw capturedMinimum;
static JoystickRaw capturedMaximum;

// Command timing.
static uint32_t lastCommandTime = 0;

// Autopilot override state.
static bool manualArmed = false;
static bool manualModeRequested = false;

// Persistent control settings and validated joystick calibration (ESP32 NVS).
static Preferences joystickPrefs;
static bool axisSetValid(const AxisCalibration &a)
{
    return a.minimum >= ADC_MINIMUM && a.maximum <= ADC_MAXIMUM &&
           a.centre > a.minimum && a.centre < a.maximum &&
           a.centre-a.minimum >= MINIMUM_AXIS_SPAN &&
           a.maximum-a.centre >= MINIMUM_AXIS_SPAN;
}
static bool fullCalibrationValid(const JoystickCalibration &v)
{
    return axisSetValid(v.leftX)&&axisSetValid(v.leftY)&&axisSetValid(v.rightX)&&axisSetValid(v.rightY)&&v.deadZone>=10&&v.deadZone<=120;
}
static void saveCalibrationNvs()
{
    joystickPrefs.begin("flytec-joy",false);
    joystickPrefs.putInt("lxmin",calibration.leftX.minimum); joystickPrefs.putInt("lxctr",calibration.leftX.centre); joystickPrefs.putInt("lxmax",calibration.leftX.maximum);
    joystickPrefs.putInt("lymin",calibration.leftY.minimum); joystickPrefs.putInt("lyctr",calibration.leftY.centre); joystickPrefs.putInt("lymax",calibration.leftY.maximum);
    joystickPrefs.putInt("rxmin",calibration.rightX.minimum); joystickPrefs.putInt("rxctr",calibration.rightX.centre); joystickPrefs.putInt("rxmax",calibration.rightX.maximum);
    joystickPrefs.putInt("rymin",calibration.rightY.minimum); joystickPrefs.putInt("ryctr",calibration.rightY.centre); joystickPrefs.putInt("rymax",calibration.rightY.maximum);
    joystickPrefs.putInt("dead",calibration.deadZone);
    joystickPrefs.putUChar("layout",(uint8_t)selectedLayout); joystickPrefs.putUChar("override",(uint8_t)selectedOverride);
    joystickPrefs.end();
}
static void loadCalibrationNvs()
{
    joystickPrefs.begin("flytec-joy",true);
    const bool saved=joystickPrefs.getBool("valid",false);
    if(saved){
      calibration.leftX={joystickPrefs.getInt("lxmin",0),joystickPrefs.getInt("lxctr",2048),joystickPrefs.getInt("lxmax",4095)};
      calibration.leftY={joystickPrefs.getInt("lymin",0),joystickPrefs.getInt("lyctr",2048),joystickPrefs.getInt("lymax",4095)};
      calibration.rightX={joystickPrefs.getInt("rxmin",0),joystickPrefs.getInt("rxctr",2048),joystickPrefs.getInt("rxmax",4095)};
      calibration.rightY={joystickPrefs.getInt("rymin",0),joystickPrefs.getInt("ryctr",2048),joystickPrefs.getInt("rymax",4095)};
    }
    calibration.deadZone=constrain(joystickPrefs.getInt("dead",65),10,120);
    const uint8_t layout=joystickPrefs.getUChar("layout",0),ov=joystickPrefs.getUChar("override",0);
    joystickPrefs.end();
    if(layout<=1)selectedLayout=(ControlLayout)layout;
    if(ov<=1)selectedOverride=(AutopilotOverride)ov;
    calibrationValid=saved&&fullCalibrationValid(calibration);
    if(!calibrationValid) calibration={{0,2048,4095},{0,2048,4095},{0,2048,4095},{0,2048,4095},calibration.deadZone};
}

// --------------------------------------------------
// RAW VALUE LIMITING
// --------------------------------------------------

static int limitRaw(int value)
{
    return constrain(value, ADC_MINIMUM, ADC_MAXIMUM);
}

// --------------------------------------------------
// NORMALISE ONE AXIS
// --------------------------------------------------

static int16_t normaliseAxis(
    int raw,
    const AxisCalibration &axis,
    int deadZone)
{
    // Limit the incoming ADC value.
    raw = limitRaw(raw);

    // Calculate the positive range after the dead zone.
    int positiveRange = axis.maximum - axis.centre - deadZone;

    // Calculate the negative range after the dead zone.
    int negativeRange = axis.centre - axis.minimum - deadZone;

    // Reject invalid calibration.
    if (positiveRange <= 0 || negativeRange <= 0)
    {
        return 0;
    }

    // Calculate the distance from the centre.
    int difference = raw - axis.centre;

    // Ignore small movements around the centre.
    if (abs(difference) <= deadZone)
    {
        return 0;
    }

    float result = 0.0f;

    if (difference > 0)
    {
        // Scale positive movement.
        result = (float)(difference - deadZone) /
                 (float)positiveRange;
    }
    else
    {
        // Scale negative movement.
        result = (float)(difference + deadZone) /
                 (float)negativeRange;
    }

    // Restrict the result to -1.0 through +1.0.
    result = constrain(result, -1.0f, 1.0f);

    // Convert to -1000 through +1000.
    return (int16_t)(result * 1000.0f);
}

// --------------------------------------------------
// NORMALISE ALL FOUR AXES
// --------------------------------------------------

static JoystickValues normaliseAll(const JoystickRaw &raw)
{
    JoystickValues result;

    // Convert left horizontal axis.
    result.leftX = normaliseAxis(
        raw.leftX,
        calibration.leftX,
        calibration.deadZone
    );

    // Convert left vertical axis.
    result.leftY = normaliseAxis(
        raw.leftY,
        calibration.leftY,
        calibration.deadZone
    );

    // Convert right horizontal axis.
    result.rightX = normaliseAxis(
        raw.rightX,
        calibration.rightX,
        calibration.deadZone
    );

    // Convert right vertical axis.
    result.rightY = normaliseAxis(
        raw.rightY,
        calibration.rightY,
        calibration.deadZone
    );

    return result;
}

// --------------------------------------------------
// NEUTRAL CHECK
// --------------------------------------------------

static bool valuesAreNeutral(const JoystickValues &v)
{
    // All four axes must be inside this normalised range.
    const int limit = 90;

    return abs(v.leftX) <= limit &&
           abs(v.leftY) <= limit &&
           abs(v.rightX) <= limit &&
           abs(v.rightY) <= limit;
}

// --------------------------------------------------
// UPDATE ENDPOINT CAPTURE
// --------------------------------------------------

static void updateEndpointCapture(const JoystickRaw &raw)
{
    // Record the smallest left X reading.
    capturedMinimum.leftX =
        min(capturedMinimum.leftX, raw.leftX);

    // Record the largest left X reading.
    capturedMaximum.leftX =
        max(capturedMaximum.leftX, raw.leftX);

    // Record the smallest left Y reading.
    capturedMinimum.leftY =
        min(capturedMinimum.leftY, raw.leftY);

    // Record the largest left Y reading.
    capturedMaximum.leftY =
        max(capturedMaximum.leftY, raw.leftY);

    // Record the smallest right X reading.
    capturedMinimum.rightX =
        min(capturedMinimum.rightX, raw.rightX);

    // Record the largest right X reading.
    capturedMaximum.rightX =
        max(capturedMaximum.rightX, raw.rightX);

    // Record the smallest right Y reading.
    capturedMinimum.rightY =
        min(capturedMinimum.rightY, raw.rightY);

    // Record the largest right Y reading.
    capturedMaximum.rightY =
        max(capturedMaximum.rightY, raw.rightY);
        // Display the endpoints currently being measured.
snapshot.calibration.leftX.minimum = capturedMinimum.leftX;
snapshot.calibration.leftX.maximum = capturedMaximum.leftX;

snapshot.calibration.leftY.minimum = capturedMinimum.leftY;
snapshot.calibration.leftY.maximum = capturedMaximum.leftY;

snapshot.calibration.rightX.minimum = capturedMinimum.rightX;
snapshot.calibration.rightX.maximum = capturedMaximum.rightX;

snapshot.calibration.rightY.minimum = capturedMinimum.rightY;
snapshot.calibration.rightY.maximum = capturedMaximum.rightY;
}

// --------------------------------------------------
// VALIDATE ONE AXIS
// --------------------------------------------------

static bool validAxis(
    int minimum,
    int centre,
    int maximum)
{
    // The minimum must be sufficiently below centre.
    if (centre - minimum < MINIMUM_AXIS_SPAN)
    {
        return false;
    }

    // The maximum must be sufficiently above centre.
    if (maximum - centre < MINIMUM_AXIS_SPAN)
    {
        return false;
    }

    // The values must be in ADC range.
    if (minimum < ADC_MINIMUM || maximum > ADC_MAXIMUM)
    {
        return false;
    }

    return true;
}

// --------------------------------------------------
// INITIALISE JOYSTICK MODULE
// --------------------------------------------------

void joystick_begin()
{
    // Clear the latest readings.
    snapshot = {};

    // Start outside calibration mode.
    calibrationMode = false;

    // Disable endpoint capture.
    endpointCapture = false;

    // Reset autopilot override flags.
    manualArmed = false;
    manualModeRequested = false;

    // Reset command timing.
    lastCommandTime = 0;

    // Load persisted settings; invalid calibration remains fail-closed.
    loadCalibrationNvs();

    // Publish the initial calibration.
    snapshot.calibration = calibration;
    snapshot.calibrationValid = calibrationValid;
}

// --------------------------------------------------
// REGISTER HARDWARE READER
// --------------------------------------------------

void joystick_setReadCallback(JoystickReadCallback callback)
{
    // Store the function that reads physical joystick axes.
    readCallback = callback;
}

// --------------------------------------------------
// CHANGE CONTROL LAYOUT
// --------------------------------------------------

void joystick_setLayout(ControlLayout layout)
{
    // Only accept recognised layouts.
    if (layout != CONTROL_TANK &&
        layout != CONTROL_STEERING)
    {
        return;
    }

    // Do not change layout while sticks are displaced.
    if (!joystick_isNeutral())
    {
        return;
    }

    // Require a fresh neutral sample after any layout change.
    manualArmed = false;
    selectedLayout = layout;
    saveCalibrationNvs();
}

// --------------------------------------------------
// GET CONTROL LAYOUT
// --------------------------------------------------

ControlLayout joystick_getLayout()
{
    return selectedLayout;
}

// --------------------------------------------------
// CHANGE AUTOPILOT OVERRIDE
// --------------------------------------------------

void joystick_setAutopilotOverride(
    AutopilotOverride behaviour)
{
    // Validate the requested behaviour.
    if (behaviour != OVERRIDE_TEMPORARY &&
        behaviour != OVERRIDE_TO_MANUAL)
    {
        return;
    }

    selectedOverride = behaviour;
    saveCalibrationNvs();
}

// --------------------------------------------------
// GET AUTOPILOT OVERRIDE
// --------------------------------------------------

AutopilotOverride joystick_getAutopilotOverride()
{
    return selectedOverride;
}

// --------------------------------------------------
// ENTER CALIBRATION MODE
// --------------------------------------------------

bool joystick_calibrationEnter()
{
    // Calibration requires a confirmed boat emergency stop.
    if (!boatState.emergencyStop ||
        !boatState.emergencyStopConfirmed)
    {
        return false;
    }

    // Suppress all motor command generation.
    calibrationMode = true;

    // Cancel any previous endpoint capture.
    endpointCapture = false;

    // Reset override state.
    manualArmed = false;
    manualModeRequested = false;

    return true;
}

// --------------------------------------------------
// EXIT CALIBRATION MODE
// --------------------------------------------------

bool joystick_calibrationExit()
{
    // If calibration was never entered, allow normal screen navigation.
    if (!calibrationMode)
    {
        return true;
    }

    // Do not leave active calibration while a stick is displaced or unreadable.
    if (!joystick_isNeutral())
    {
        return false;
    }

    // Stop collecting endpoint readings.
    endpointCapture = false;

    // Leave calibration mode.
    calibrationMode = false;

    return true;
}

// --------------------------------------------------
// CAPTURE CENTRE POSITION
// --------------------------------------------------

bool joystick_calibrateCentre()
{
    // Centre capture is only allowed in calibration mode.
    if (!calibrationMode || readCallback == nullptr)
    {
        return false;
    }

    // Accumulate readings for all four axes.
    long sumLeftX = 0;
    long sumLeftY = 0;
    long sumRightX = 0;
    long sumRightY = 0;

    // Number of samples to average.
    const int sampleCount = 32;

    // Track sample extremes so a moving stick cannot be mistaken for centre.
    JoystickRaw sampleMin = {ADC_MAXIMUM, ADC_MAXIMUM, ADC_MAXIMUM, ADC_MAXIMUM};
    JoystickRaw sampleMax = {ADC_MINIMUM, ADC_MINIMUM, ADC_MINIMUM, ADC_MINIMUM};

    // Read multiple samples to reduce ADC noise.
    for (int i = 0; i < sampleCount; i++)
    {
        JoystickRaw raw;

        // Abort if any hardware read fails.
        if (!readCallback(raw))
        {
            return false;
        }

        // Reject impossible ADC values immediately.
        if (raw.leftX < ADC_MINIMUM || raw.leftX > ADC_MAXIMUM ||
            raw.leftY < ADC_MINIMUM || raw.leftY > ADC_MAXIMUM ||
            raw.rightX < ADC_MINIMUM || raw.rightX > ADC_MAXIMUM ||
            raw.rightY < ADC_MINIMUM || raw.rightY > ADC_MAXIMUM)
        {
            return false;
        }

        // Add the current readings.
        sumLeftX += raw.leftX;
        sumLeftY += raw.leftY;
        sumRightX += raw.rightX;
        sumRightY += raw.rightY;

        // Track the range observed during centre sampling.
        sampleMin.leftX = min(sampleMin.leftX, raw.leftX);
        sampleMin.leftY = min(sampleMin.leftY, raw.leftY);
        sampleMin.rightX = min(sampleMin.rightX, raw.rightX);
        sampleMin.rightY = min(sampleMin.rightY, raw.rightY);
        sampleMax.leftX = max(sampleMax.leftX, raw.leftX);
        sampleMax.leftY = max(sampleMax.leftY, raw.leftY);
        sampleMax.rightX = max(sampleMax.rightX, raw.rightX);
        sampleMax.rightY = max(sampleMax.rightY, raw.rightY);

        // Allow ADC readings to settle.
        delay(4);
    }

    // If an axis moves too much during sampling, reject this centre capture.
    const int maxCentreJitter = 100;
    if (sampleMax.leftX - sampleMin.leftX > maxCentreJitter ||
        sampleMax.leftY - sampleMin.leftY > maxCentreJitter ||
        sampleMax.rightX - sampleMin.rightX > maxCentreJitter ||
        sampleMax.rightY - sampleMin.rightY > maxCentreJitter)
    {
        return false;
    }

    // Calculate average centre positions.
    // Reject impossible ADC readings before accepting the centre values.
    if (sumLeftX < 0 || sumLeftY < 0 || sumRightX < 0 || sumRightY < 0 ||
        sumLeftX > (long)ADC_MAXIMUM * sampleCount ||
        sumLeftY > (long)ADC_MAXIMUM * sampleCount ||
        sumRightX > (long)ADC_MAXIMUM * sampleCount ||
        sumRightY > (long)ADC_MAXIMUM * sampleCount)
    {
        return false;
    }

    calibration.leftX.centre = sumLeftX / sampleCount;
    calibration.leftY.centre = sumLeftY / sampleCount;
    calibration.rightX.centre = sumRightX / sampleCount;
    calibration.rightY.centre = sumRightY / sampleCount;

    // A new centre invalidates the old endpoint calibration.
    calibrationValid = false;
    snapshot.calibration = calibration;
    snapshot.calibrationValid = false;
    joystickPrefs.begin("flytec-joy",false);
    joystickPrefs.putBool("valid",false);
    joystickPrefs.end();

    return true;
}

// --------------------------------------------------
// START ENDPOINT CAPTURE
// --------------------------------------------------

bool joystick_beginEndpointCapture()
{
    // Endpoint capture requires calibration mode.
    if (!calibrationMode || !readCallback)
    {
        return false;
    }

    // Read the initial positions.
    JoystickRaw raw;

    if (!readCallback(raw))
    {
        return false;
    }

    // Start each minimum at the current reading.
    capturedMinimum = raw;

    // Start each maximum at the current reading.
    capturedMaximum = raw;

    // Enable continuous endpoint recording.
    endpointCapture = true;

    return true;
}

// --------------------------------------------------
// FINISH ENDPOINT CAPTURE
// --------------------------------------------------

bool joystick_endEndpointCapture()
{
    // There must be an active capture.
    if (!calibrationMode || !endpointCapture)
    {
        return false;
    }

    // Stop collecting new readings.
    endpointCapture = false;

    // Validate the left X axis.
    bool leftXValid = validAxis(
        capturedMinimum.leftX,
        calibration.leftX.centre,
        capturedMaximum.leftX
    );

    // Validate the left Y axis.
    bool leftYValid = validAxis(
        capturedMinimum.leftY,
        calibration.leftY.centre,
        capturedMaximum.leftY
    );

    // Validate the right X axis.
    bool rightXValid = validAxis(
        capturedMinimum.rightX,
        calibration.rightX.centre,
        capturedMaximum.rightX
    );

    // Validate the right Y axis.
    bool rightYValid = validAxis(
        capturedMinimum.rightY,
        calibration.rightY.centre,
        capturedMaximum.rightY
    );

    // Reject the entire capture if any axis is invalid.
    if (!leftXValid || !leftYValid ||
        !rightXValid || !rightYValid)
    {
        snapshot.calibration = calibration;
        snapshot.calibrationValid = calibrationValid;
        return false;
    }

    // Commit the new minimum and maximum values.
    calibration.leftX.minimum = capturedMinimum.leftX;
    calibration.leftX.maximum = capturedMaximum.leftX;

    calibration.leftY.minimum = capturedMinimum.leftY;
    calibration.leftY.maximum = capturedMaximum.leftY;

    calibration.rightX.minimum = capturedMinimum.rightX;
    calibration.rightX.maximum = capturedMaximum.rightX;

    calibration.rightY.minimum = capturedMinimum.rightY;
    calibration.rightY.maximum = capturedMaximum.rightY;

    // Calibration becomes valid only after all four axes pass validation.
    calibrationValid = true;
    snapshot.calibration = calibration;
    snapshot.calibrationValid = true;
    joystickPrefs.begin("flytec-joy",false);
    joystickPrefs.putBool("valid",true);
    joystickPrefs.end();
    saveCalibrationNvs();

    return true;
}

// --------------------------------------------------
// SET DEAD ZONE
// --------------------------------------------------

void joystick_setDeadZone(int deadZone)
{
    // Match the calibration screen slider: 10 to 120 ADC counts.
    calibration.deadZone = constrain(deadZone, 10, 120);
    snapshot.calibration = calibration;
    saveCalibrationNvs();
}

// --------------------------------------------------
// RETURN CURRENT SNAPSHOT
// --------------------------------------------------

JoystickSnapshot joystick_getSnapshot()
{
    // Return a copy of the latest readings and settings.
    JoystickSnapshot result = snapshot;

    // Include current calibration flags.
    result.calibrationMode = calibrationMode;
    result.calibrationValid = calibrationValid;
    result.endpointCapture = endpointCapture;

    return result;
}

// --------------------------------------------------
// CHECK NEUTRAL POSITION
// --------------------------------------------------

bool joystick_isNeutral()
{
    // Invalid readings cannot be considered neutral.
    if (!snapshot.readingsValid)
    {
        return false;
    }

    return snapshot.neutral;
}

// --------------------------------------------------
// GET NORMALISED VALUES
// --------------------------------------------------

JoystickValues joystick_getValues()
{
    return snapshot.values;
}

// --------------------------------------------------
// MAIN JOYSTICK UPDATE
// --------------------------------------------------

void joystick_update()
{
    // A valid hardware callback is required.
    if (readCallback == nullptr)
    {
        manualArmed = false;
        snapshot.readingsValid = false;
        snapshot.neutral = false;
        return;
    }

    // Read all four physical axes.
    JoystickRaw raw;

    if (!readCallback(raw))
    {
        // Mark readings invalid.
        manualArmed = false;
        snapshot.readingsValid = false;
        snapshot.neutral = false;

        // Never send a motor command after a failed read.
        return;
    }

    // Reject ADC values outside the expected range.
    if (raw.leftX < ADC_MINIMUM || raw.leftX > ADC_MAXIMUM ||
        raw.leftY < ADC_MINIMUM || raw.leftY > ADC_MAXIMUM ||
        raw.rightX < ADC_MINIMUM || raw.rightX > ADC_MAXIMUM ||
        raw.rightY < ADC_MINIMUM || raw.rightY > ADC_MAXIMUM)
    {
        manualArmed = false;
        snapshot.readingsValid = false;
        snapshot.neutral = false;
        return;
    }

    // Store raw readings.
    snapshot.raw = raw;

    // Convert raw values to motor-control values.
    snapshot.values = normaliseAll(raw);

    // Mark the readings valid.
    snapshot.readingsValid = true;

    // Calculate neutral status.
    snapshot.neutral = valuesAreNeutral(snapshot.values);

    // Publish current calibration.
    snapshot.calibration = calibration;

    // Collect endpoint readings when requested.
    if (endpointCapture)
    {
        updateEndpointCapture(raw);
    }

    // --------------------------------------------------
    // SAFETY GATES
    // No autonomous override until the Boat mission executor acknowledges it.
    if(calibrationMode || !calibrationValid || !boatRadio_isConnected() ||
       !boatState.radioConnected || boatState.emergencyStop ||
       !boatState.emergencyStopConfirmed || boatState.failsafeActive ||
       !boatState.actuatorSafetyReady)
    {
        manualArmed=false;manualModeRequested=false;return;
    }
    if(boatState.mode!=MODE_MANUAL)
    {
        manualArmed=false;
        // Switch-to-manual is a request only; wait for confirmed manual mode.
        if(boatState.mode==MODE_AUTOPILOT && selectedOverride==OVERRIDE_TO_MANUAL &&
           !snapshot.neutral && !manualModeRequested)
            manualModeRequested=boatCommands_setMode(MODE_MANUAL);
        if(snapshot.neutral)manualModeRequested=false;
        return;
    }
    manualModeRequested=false;
    // After startup, STOP, calibration, radio fault or mode change, require
    // neutral sticks before a subsequent displacement can command movement.
    if(!manualArmed){
        if(snapshot.neutral)manualArmed=true;
        return;
    }

    // --------------------------------------------------
    // COMMAND RATE LIMIT
    // --------------------------------------------------

    uint32_t now = millis();

    if (now - lastCommandTime < COMMAND_INTERVAL_MS)
    {
        return;
    }

    lastCommandTime = now;

    // --------------------------------------------------
    // CALCULATE MOTOR OUTPUTS
    // --------------------------------------------------

    int16_t leftMotor = 0;
    int16_t rightMotor = 0;

    if (selectedLayout == CONTROL_TANK)
    {
        // Left stick controls the left motor.
        leftMotor = -snapshot.values.leftY;

        // Right stick controls the right motor.
        rightMotor = -snapshot.values.rightY;
    }
    else
    {
        // Left Y controls throttle.
        int16_t throttle = -snapshot.values.leftY;

        // Right X controls steering.
        int16_t steering = snapshot.values.rightX;

        // Mix throttle and steering.
        leftMotor = throttle + steering;
        rightMotor = throttle - steering;

        // Restrict both outputs.
        leftMotor = constrain(leftMotor, -1000, 1000);
        rightMotor = constrain(rightMotor, -1000, 1000);
    }

    // Failed transmission requires a fresh neutral sample before retrying motion.
    if(!boatCommands_setManualMotors(leftMotor, rightMotor))manualArmed=false;
}
