#pragma once
#include "boat_commands.h"
#include <stdint.h>

enum BoatPolicyResult : uint8_t {
    BOAT_POLICY_ACCEPTED=0,
    BOAT_POLICY_REJECTED_SAFETY,
    BOAT_POLICY_INVALID
};

struct BoatPolicyInputs {
    bool primaryLinkFresh;
    bool backupLinkFresh;
    bool physicalStopReleased;
    bool batteryHealthy;
    bool motorsNeutral;
    bool gnssFresh;
    bool headingFresh;
    bool geofenceValid;
    bool homeValid;
    bool missionTargetValid;
    bool releaseZoneReached;
    bool releaseInterlocksOk;
};

struct BoatPolicyState {
    BoatMode mode;
    bool stopLatched;
    bool missionActive;
};

struct BoatPolicyOutput {
    int16_t leftMotor;
    int16_t rightMotor;
    int8_t releaseChannel; // 0..3 identifies independent release; -1 means none
    bool requestReturnHome;
};

void boatPolicyInit(BoatPolicyState &state);
BoatPolicyResult boatPolicyApply(const BoatCommand &cmd,bool receivedOnBackup,
                                const BoatPolicyInputs &inputs,
                                BoatPolicyState &state,BoatPolicyOutput &out);
