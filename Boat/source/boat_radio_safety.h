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

enum BoatCrossRadioCommandDecision : uint8_t {\n    BOAT_CROSSRADIO_NEW_COMMAND = 0,\n    BOAT_CROSSRADIO_DUPLICATE_EXECUTED,\n    BOAT_CROSSRADIO_DUPLICATE_PENDING,\n    BOAT_CROSSRADIO_DROP_STALE\n};\n\n// Only a sequence-rejected command may pass if its exact command ID is already\n// known pending/executed. A stale sequence with a new ID is always dropped.\nBoatCrossRadioCommandDecision boatClassifyCrossRadioCommand(bool sequenceAccepted,\n                                                            bool alreadyExecuted,\n                                                            bool pending);\n\n// Evaluates only; it does not drive motors or actuators. Call from the boat's
// independent safety supervisor. Link loss must never enable solenoid outputs.
BoatFailsafeDecision boatEvaluateLinkFailsafe(const BoatLinkSafetyInputs &in);
