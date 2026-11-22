
#include "boat_state.h"

// Create the shared boat state variable.
BoatState boatState;

// Initialise all values before starting the application.
void initialiseBoatState()
{
    boatState.mode = MODE_MANUAL;

    boatState.radioConnected = false;
    boatState.radioPrimaryConnected = false;
    boatState.radioBackupConnected = false;
    boatState.radioActiveLink = RADIO_UI_NONE;
    boatState.radioLinkState = RADIO_UI_DISCONNECTED;
    boatState.gpsValid = false;
    boatState.gpsFixType = 0;
    boatState.gpsSatellites = 0;
    boatState.gpsAccuracyMeters = 0.0f;
    boatState.gpsLastUpdateMs = 0;
    boatState.sonarActive = false;
    boatState.headingValid = false;
    boatState.lowBattery = false;
    boatState.failsafeActive = false;
    boatState.sdCardReady = false;
    boatState.offlineMapReady = false;

    boatState.batteryVoltage = 0.0f;
    boatState.batteryPercent = 0.0f;

    boatState.latitude = 0.0f;
    boatState.longitude = 0.0f;

    boatState.heading = 0.0f;
    boatState.speed = 0.0f;

    boatState.motorLeftActive = false;
    boatState.motorRightActive = false;
    // Start in a latched-safe state until telemetry confirms boat status.
    boatState.emergencyStop = true;
    boatState.emergencyStopConfirmed = false;
    boatState.actuatorSafetyReady = false;
    boatState.diagnosticFresh = false;
    boatState.diagnosticReason = 0;
    boatState.diagnosticFlags = 0;
}
