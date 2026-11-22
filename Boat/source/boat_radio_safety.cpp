#include "boat_radio_safety.h"
#include <string.h>

BoatCommandDeduplicator::BoatCommandDeduplicator() : writeIndex_(0) { clear(); }
void BoatCommandDeduplicator::clear() {
    memset(entries_,0,sizeof(entries_));
    writeIndex_=0;
}
bool BoatCommandDeduplicator::hasExecuted(uint8_t sender, uint8_t epoch, uint16_t id) const {
    for (const Entry &e : entries_)
        if (e.used && e.sender==sender && e.epoch==epoch && e.id==id) return true;
    return false;
}
void BoatCommandDeduplicator::markExecuted(uint8_t sender, uint8_t epoch, uint16_t id) {
    if (hasExecuted(sender,epoch,id)) return;
    entries_[writeIndex_]={sender,epoch,id,true};
    writeIndex_=(uint8_t)((writeIndex_+1U)%32U);
}
BoatFailsafeDecision boatEvaluateLinkFailsafe(const BoatLinkSafetyInputs &in) {
    // Backup alone is not continuous manual authority: LoRa pacing exceeds
    // the Boat's 200ms motor command lease. Only primary freshness suppresses
    // link-loss action; backup remains available for STOP/Home requests.
    if (in.nrfValidFresh) return BOAT_FS_NONE;
    if (in.autonomousMissionActive && in.gnssValidFresh && in.headingValid &&
        in.batteryHealthy && in.geofenceValid)
        return BOAT_FS_RETURN_HOME;
    return BOAT_FS_STOP_PROPULSION;
}

BoatCrossRadioCommandDecision boatClassifyCrossRadioCommand(bool sequenceAccepted,
                                                            bool alreadyExecuted,
                                                            bool pending) {
    if(sequenceAccepted) return BOAT_CROSSRADIO_NEW_COMMAND;
    if(alreadyExecuted) return BOAT_CROSSRADIO_DUPLICATE_EXECUTED;
    if(pending) return BOAT_CROSSRADIO_DUPLICATE_PENDING;
    return BOAT_CROSSRADIO_DROP_STALE;
}
