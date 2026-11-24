#pragma once
// R30H-19: latest-authoritative-status priority outbox (host portable).
// The caller MUST supply a real locally sampled status. This class never
// invents STOP, changes an actuator output, or attests physical current zero.
#include <stdint.h>
#include "radio_payload_codec.h"
namespace boat_r30h19 {
class StatusOutbox final {
public:
    static constexpr uint32_t MAX_STATUS_AGE_MS=300U;
    void reset(){pending_=false;offeredAt_=0;status_={};advanceRevision();}
    bool offer(const RadioDecodedPayload &status,uint32_t now){
        // Mirror the strict 8-byte on-wire STATUS encoder contract. Invalid
        // snapshots must not repeatedly occupy the priority TX scheduler.
        if(status.kind!=RADIO_PAYLOAD_STATUS||status.stopState>2U||
           status.boatMode>4U||(status.safetyFlags&0x80U)!=0U||
           (status.batteryPercent>100U&&status.batteryPercent!=255U)){
            // Invalid/newer feedback revokes a previously queued stopped
            // snapshot. Never send a stale STOP witness after a failed read.
            reset();
            return false;
        }
        // A replacement snapshot invalidates any older asynchronous reservation.
        advanceRevision();
        if(revision_==UINT64_MAX){pending_=false;return false;}
        status_=status;offeredAt_=now;pending_=true;return true;
    }
    bool pending()const{return pending_;}
    bool due(uint32_t now,bool sessionReady,bool primaryReady,bool txBusy,
             bool stopAckPending){
        if(!pending_)return false;
        // Stale or untrusted status is never replayed after a session/PHY
        // change; STOP ACK must be transmitted first.
        if(!sessionReady||!primaryReady||
           (uint32_t)(now-offeredAt_)>MAX_STATUS_AGE_MS){reset();return false;}
        return !txBusy&&!stopAckPending;
    }
    uint64_t revision()const{return revision_;}
    uint32_t remainingAge(uint32_t now)const{
        const uint32_t elapsed=uint32_t(now-offeredAt_);
        // Preserve the existing inclusive 300 ms freshness boundary.
        return pending_&&elapsed<=MAX_STATUS_AGE_MS?MAX_STATUS_AGE_MS+1U-elapsed:0;
    }
    bool sentRevision(uint64_t revision){if(!pending_||revision!=revision_)return false;reset();return true;}
    const RadioDecodedPayload &status()const{return status_;}
    void sent(){reset();} // radio TX admission only, NOT remote receipt
private:
    uint64_t revision_=0;
    void advanceRevision(){if(revision_!=UINT64_MAX)++revision_;}
    RadioDecodedPayload status_{};
    uint32_t offeredAt_=0;
    bool pending_=false;
};
}
