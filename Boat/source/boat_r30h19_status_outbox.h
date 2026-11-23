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
    void reset(){pending_=false;offeredAt_=0;status_={};}
    bool offer(const RadioDecodedPayload &status,uint32_t now){
        // Mirror the strict 8-byte on-wire STATUS encoder contract. Invalid
        // snapshots must not repeatedly occupy the priority TX scheduler.
        if(status.kind!=RADIO_PAYLOAD_STATUS||status.stopState>2U||
           status.boatMode>4U||(status.safetyFlags&0x80U)!=0U||
           (status.batteryPercent>100U&&status.batteryPercent!=255U))
            return false;
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
    const RadioDecodedPayload &status()const{return status_;}
    void sent(){reset();} // radio TX admission only, NOT remote receipt
private:
    RadioDecodedPayload status_{};
    uint32_t offeredAt_=0;
    bool pending_=false;
};
}
