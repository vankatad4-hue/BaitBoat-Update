#pragma once
#include <Arduino.h>

enum BoatFailsafeDecision : uint8_t {
    BOAT_FS_NONE = 0,
    BOAT_FS_STOP_PROPULSION,
    BOAT_FS_RETURN_HOME
};

struct BoatLinkSafetyInputs {
    bool nrfValidFresh;
    bool loraValidFresh;
    bool autonomousMissionActive;
    bool gnssValidFresh;
    bool headingValid;
    bool batteryHealthy;
    bool geofenceValid;
};

class BoatCommandDeduplicator {
public:
    BoatCommandDeduplicator();
    bool hasExecuted(uint8_t senderId, uint8_t sessionEpoch, uint16_t commandId) const;
    void markExecuted(uint8_t senderId, uint8_t sessionEpoch, uint16_t commandId);
    void clear();
private:
    struct Entry { uint8_t sender; uint8_t epoch; uint16_t id; bool used; };
    Entry entries_[32];
    uint8_t writeIndex_;
};

// Evaluates only; it does not drive motors or actuators. Call from the boat's
// independent safety supervisor. Link loss must never enable solenoid outputs.
BoatFailsafeDecision boatEvaluateLinkFailsafe(const BoatLinkSafetyInputs &in);
