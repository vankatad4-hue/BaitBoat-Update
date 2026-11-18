
#ifndef BOAT_STATE_H
#define BOAT_STATE_H

#include <Arduino.h>

// Operating modes available on the remote.
enum BoatMode
{
    MODE_MANUAL,       // Direct joystick control.
    MODE_AUTOPILOT,    // Automatic navigation.
    MODE_RETURN_HOME   // Return to stored home position.
};

// Current state of the boat.
struct BoatState
{
    BoatMode mode;         // Current operating mode.

    bool radioConnected;   // Radio communication status.
    bool gpsValid;         // True only while the latest GPS fix is valid and fresh.
    uint8_t gpsFixType;    // GNSS fix type reported by the boat (0=no fix, 3=3D fix, etc.).
    uint8_t gpsSatellites; // Number of satellites used in the solution.
    float gpsAccuracyMeters; // Estimated horizontal position accuracy in metres.
    uint32_t gpsLastUpdateMs; // Local receipt time of the latest valid GPS telemetry.
    bool sonarActive;      // Sonar operating status.

    bool sdCardReady;      // True when the remote TF card is mounted.
    bool offlineMapReady;  // True when the offline-map manifest is installed.

    float batteryVoltage;  // Boat battery voltage.
    float batteryPercent;  // Estimated battery charge.

    float latitude;        // Current GPS latitude.
    float longitude;       // Current GPS longitude.

    float heading;         // Boat compass heading in degrees.
    float speed;           // Boat speed in metres per second.

    bool motorLeftActive;  // Left motor status.
    bool motorRightActive; // Right motor status.

    bool emergencyStop;             // True when the boat reports STOP active.
    bool emergencyStopConfirmed;    // True only when telemetry confirms the current emergency-stop state (active or released).
};

// Global boat state shared between software modules.
extern BoatState boatState;

// Initialise the boat state with safe default values.
void initialiseBoatState();
#endif
