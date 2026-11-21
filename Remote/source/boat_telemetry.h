#pragma once
#include <Arduino.h>
#include "boat_state.h"
#include "radio_payload_codec.h"
struct BoatTelemetry {
 float batteryVoltage;int batteryPercent;float latitude,longitude,heading,speed;
 bool gpsConnected;uint8_t gpsFixType,gpsSatellites;float gpsAccuracyMeters;
 bool radioConnected,autopilotActive,motorsActive;uint32_t receivedAtMs;
 bool statusValid,statusPrimary;uint8_t reportedMode,stopState,safetyFlags;uint32_t statusReceivedAtMs;
 bool batteryVoltageValid,batteryPercentValid;uint32_t batteryReceivedAtMs,batteryPercentReceivedAtMs;uint16_t batteryMillivolts;
 bool latitudeRecordValid,longitudeRecordValid,coordinatePairValid;uint8_t latitudeSample,longitudeSample;
 uint32_t latitudeReceivedAtMs,longitudeReceivedAtMs,coordinatePairReceivedAtMs;
 bool navigationValid;uint8_t navigationStatus,waypointIndex,navigationSample;uint32_t navigationReceivedAtMs;
 bool weatherValid=false;int16_t temperatureCentiC=0;uint32_t pressurePa=0,weatherReceivedAtMs=0;
 bool sonarSummaryValid;uint16_t sonarDepthCm,sonarRangeCm;uint8_t sonarHealth,sonarAlarmFlags,sonarSample;uint32_t sonarReceivedAtMs;
};
extern BoatTelemetry telemetry;
void telemetryInit();void telemetryUpdate(const BoatTelemetry &newData);
bool telemetryApplyRadioPayload(const RadioDecodedPayload &payload,bool primaryLink,uint32_t nowMs);
void telemetryApplyToBoatState(BoatState &state,uint32_t nowMs);
