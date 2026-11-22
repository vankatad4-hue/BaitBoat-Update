#pragma once
#include <Arduino.h>

enum BoatFailsafeDecision : uint8_t {
    BOAT_FS_NONE = 0,
    BOAT_FS_STOP_ALL,
    // Compatibility alias for older callers; link loss now stops propulsion
    // and every solenoid output, not just the motors.
    BOAT_FS_STOP_PROPULSION = BOAT_FS_STOP_ALL,
    // Retained for explicit, locally validated Home requests only. The radio
    // watchdog never selects automatic Return-to-Home on link loss.
    BOAT_FS_RETURN_HOME
};

enum BoatRadioWatchdogEvent : uint8_t {
    BOAT_RADIO_WATCHDOG_NO_CHANGE = 0,
    BOAT_RADIO_WATCHDOG_LOST,
    BOAT_RADIO_WATCHDOG_RESTORED
};

// Software watchdog over authenticated/fresh link status supplied by the
// radio session manager. It is not an independent hardware cutoff.
class BoatRadioWatchdog {
public:
    BoatRadioWatchdogEvent update(bool primaryFresh, bool backupFresh) {
        const bool nextAlive = primaryFresh || backupFresh;
        if (nextAlive == alive_) return BOAT_RADIO_WATCHDOG_NO_CHANGE;
        alive_ = nextAlive;
        return alive_ ? BOAT_RADIO_WATCHDOG_RESTORED : BOAT_RADIO_WATCHDOG_LOST;
    }
    bool alive() const { return alive_; }
    void reset() { alive_ = false; }
private:
    bool alive_ = false;
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

enum BoatCrossRadioCommandDecision : uint8_t {
    BOAT_CROSSRADIO_NEW_COMMAND = 0,
    BOAT_CROSSRADIO_DUPLICATE_EXECUTED,
    BOAT_CROSSRADIO_DUPLICATE_PENDING,
    BOAT_CROSSRADIO_DROP_STALE
};

// Only a sequence-rejected command may pass if its exact command ID is already
// known pending/executed. A stale sequence with a new ID is always dropped.
BoatCrossRadioCommandDecision boatClassifyCrossRadioCommand(bool sequenceAccepted,
                                                            bool alreadyExecuted,
                                                            bool pending);

// Evaluates only; it does not drive motors or actuators. This is software
// policy in the main controller, not an independent hardware safety channel.
BoatFailsafeDecision boatEvaluateLinkFailsafe(const BoatLinkSafetyInputs &in);
