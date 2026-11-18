#pragma once

#include <Arduino.h>
#include "boat_state.h"

// Structure containing the latest boat telemetry
struct BoatTelemetry
{
    float batteryVoltage;
    int batteryPercent;

    float latitude;
    float longitude;

    float heading;
    float speed;

    bool gpsConnected;
    uint8_t gpsFixType;
    uint8_t gpsSatellites;
    float gpsAccuracyMeters;
    bool radioConnected;

    bool autopilotActive;
    bool motorsActive;

    // Local receipt timestamp; this field is not part of the radio wire packet.
    uint32_t receivedAtMs;
};

// Global telemetry data
extern BoatTelemetry telemetry;

// Initialise telemetry values
void telemetryInit();

// Update telemetry with new values
void telemetryUpdate(const BoatTelemetry &newData);

// Copy validated telemetry into the shared UI state and expire stale GPS fixes.
void telemetryApplyToBoatState(BoatState &state);