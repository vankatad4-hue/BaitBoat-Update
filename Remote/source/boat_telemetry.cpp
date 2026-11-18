#include "boat_telemetry.h"

// Create the global telemetry object
BoatTelemetry telemetry;

// Set initial values
void telemetryInit()
{
    telemetry.batteryVoltage = 0.0;
    telemetry.batteryPercent = 0;

    telemetry.latitude = 0.0;
    telemetry.longitude = 0.0;

    telemetry.heading = 0.0;
    telemetry.speed = 0.0;

    telemetry.gpsConnected = false;
    telemetry.gpsFixType = 0;
    telemetry.gpsSatellites = 0;
    telemetry.gpsAccuracyMeters = 0.0f;
    telemetry.radioConnected = false;

    telemetry.autopilotActive = false;
    telemetry.motorsActive = false;
    telemetry.receivedAtMs = 0;
}

// Copy received telemetry into the global structure
void telemetryUpdate(const BoatTelemetry &newData)
{
    telemetry = newData;
    telemetry.receivedAtMs = millis();
}

// Apply the most recent boat telemetry to the shared remote-control state.
void telemetryApplyToBoatState(BoatState &state)
{
    // Copy link and battery information from the last received boat packet.
    state.radioConnected = telemetry.radioConnected;
    state.batteryVoltage = telemetry.batteryVoltage;
    state.batteryPercent = telemetry.batteryPercent;

    // Copy navigation values even when the fix is invalid for diagnostics.
    state.latitude = telemetry.latitude;
    state.longitude = telemetry.longitude;
    state.heading = telemetry.heading;
    state.speed = telemetry.speed;
    state.gpsFixType = telemetry.gpsFixType;
    state.gpsSatellites = telemetry.gpsSatellites;
    state.gpsAccuracyMeters = telemetry.gpsAccuracyMeters;
    state.gpsLastUpdateMs = telemetry.receivedAtMs;

    // A usable position requires a 3D fix, valid coordinates and recent telemetry.
    const bool coordinatesValid =
        telemetry.latitude >= -90.0f && telemetry.latitude <= 90.0f &&
        telemetry.longitude >= -180.0f && telemetry.longitude <= 180.0f;
    const bool telemetryFresh =
        telemetry.receivedAtMs != 0 &&
        static_cast<uint32_t>(millis() - telemetry.receivedAtMs) <= 2000U;

    state.gpsValid = telemetry.radioConnected &&
                     telemetry.gpsConnected &&
                     telemetry.gpsFixType >= 3 &&
                     coordinatesValid &&
                     telemetryFresh;

    // Do not infer heading validity from GPS position alone. Compass
    // calibration and heading-quality flags will be added to the protocol.
    // This GPS update path deliberately does not change operating mode or STOP.
}