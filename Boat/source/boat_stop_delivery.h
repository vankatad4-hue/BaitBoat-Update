#pragma once
// Consolidated STOP delivery policy; original types, namespaces and limits retained.
// R30H-18: one-slot, STOP-only primary ACK outbox. Logic-only; no GPIO/RF.
// Driver TX admission is NOT application ACK delivery or physical STOP proof.
#include <stdint.h>
namespace boat_r30h18 {
struct StopAck {
    uint16_t commandId=0;
    uint8_t epoch=0;
    uint8_t result=0;
    uint8_t stopState=0;
    uint8_t interlocks=0;
};
template<uint32_t AgeMs=2500U>
class BasicStopAckOutbox final {
public:
    static constexpr uint32_t MAX_AGE_MS=AgeMs;
    static constexpr uint32_t RETRY_GAP_MS=75U;
    static constexpr uint8_t MAX_TX_ATTEMPTS=8U;
    void reset(){ticket_={};pending_=false;inFlight_=false;attempts_=0;queuedAt_=lastTryAt_=0;}
    // A new STOP replaces the old slot. Repeated STOP with the same ID/epoch
    // keeps its original deadline, even if RF command retries keep arriving.
    bool offer(uint16_t id,uint8_t epoch,uint8_t result,uint8_t stopState,
               uint8_t interlocks,uint32_t now){
        if(!id||!epoch)return false;
        if(pending_&&ticket_.commandId==id&&ticket_.epoch==epoch){
            // Only promote ACCEPTED -> EXECUTED; never downgrade it.
            if(result==1U){ticket_.result=result;ticket_.stopState=stopState;ticket_.interlocks=interlocks;}
            return true;
        }
        ticket_={id,epoch,result,stopState,interlocks};
        pending_=true;inFlight_=false;attempts_=0;queuedAt_=now;lastTryAt_=0;
        return true;
    }
    bool pending()const{return pending_;}
    bool inFlight()const{return inFlight_;}
    uint8_t attempts()const{return attempts_;}
    StopAck ticket()const{return ticket_;}
    uint32_t remainingAge(uint32_t now)const{
        const uint32_t elapsed=uint32_t(now-queuedAt_);
        return pending_&&elapsed<AgeMs?AgeMs-elapsed:0;
    }
    // Expire even when the SPI transport cannot accept a frame. Never replay
    // across a session, primary driver failure or a newer STOP command.
    bool due(uint32_t now,bool sessionReady,bool primaryReady,bool txBusy){
        if(!pending_)return false;
        if(!sessionReady||!primaryReady||(uint32_t)(now-queuedAt_)>=AgeMs){reset();return false;}
        return !txBusy&&!inFlight_&&attempts_<MAX_TX_ATTEMPTS&&
               (!attempts_||(uint32_t)(now-lastTryAt_)>=RETRY_GAP_MS);
    }
    void admitted(uint32_t now){
        if(!pending_||inFlight_||attempts_>=MAX_TX_ATTEMPTS)return;
        ++attempts_;lastTryAt_=now;inFlight_=true;
    }
    // RF24 TX_DS means hardware-level success, not Remote application ACK.
    // On-air failure retries are bounded; Remote STOP retries cover a lost
    // application ACK even after the Boat's hardware reports success.
    void completed(bool radioTxSuccess){
        if(!pending_||!inFlight_)return;
        inFlight_=false;
        if(radioTxSuccess||attempts_>=MAX_TX_ATTEMPTS)reset();
    }
private:
    StopAck ticket_{};
    bool pending_=false,inFlight_=false;
    uint8_t attempts_=0;
    uint32_t queuedAt_=0,lastTryAt_=0;
};
using StopAckOutbox=BasicStopAckOutbox<>;
using BackupStopAckOutbox=BasicStopAckOutbox<12000U>;
} // namespace boat_r30h18

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

// R30H-21: primary nRF ordinary traffic admission before allocating a
// 16-bit authenticated packet sequence number. Pure C++ (no radio/GPIO).
// STOP ACK and fresh STATUS have a separate high-priority scheduler and
// are NEVER admitted through this ordinary-frame policy.
namespace boat_r30h21 {
struct PrimaryTxConditions {
    bool sessionReady=false;
    bool radioReady=false;
    bool txBusy=false;
    bool stopAckPending=false;
    bool statusPending=false;
};
inline bool ordinarySlotFree(const PrimaryTxConditions &s) {
    return s.sessionReady && s.radioReady && !s.txBusy &&
           !s.stopAckPending && !s.statusPending;
}
} // namespace boat_r30h21

// Dormant worker-only producer. Cadence bounds new reservations, not RF
// transmissions. No modification to legacy scheduling or airtime approval.
namespace boat_r30h19 {
class BackupStatusCadence {
 bool offered_=false,stopped_=false;
 uint32_t at_=0;
 static bool valid(const RadioDecodedPayload& s){
  return s.kind==RADIO_PAYLOAD_STATUS&&s.stopState<=2U&&s.boatMode<=4U&&
   !(s.safetyFlags&0x80U)&&(s.batteryPercent<=100U||s.batteryPercent==255U);
 }
 static bool same(const RadioDecodedPayload& a,const RadioDecodedPayload& b){
  return a.boatMode==b.boatMode&&a.stopState==b.stopState&&
   a.safetyFlags==b.safetyFlags&&a.batteryPercent==b.batteryPercent;
 }
public:
 static constexpr uint32_t INTERVAL_MS=3000U;
 void reset(){offered_=stopped_=false;at_=0;}
 bool offer(StatusOutbox& box,const RadioDecodedPayload& sample,uint32_t sampledAt,uint32_t now,
            bool trusted,bool tracking,bool correlatedMission){
  if(!trusted||!valid(sample)||uint32_t(now-sampledAt)>StatusOutbox::MAX_STATUS_AGE_MS){
   if(box.pending())box.reset();
   return false;
  }
  const bool stopped=sample.stopState==1U;
  const bool newStop=stopped&&!stopped_;
  // Preserve a newly observed STOP transition until it can be reserved.
  if(!stopped)stopped_=false;
  if(correlatedMission&&!stopped){if(box.pending())box.reset();return false;}
  if(box.pending()){
   if(tracking){if(!same(box.status(),sample))box.reset();return false;}
   const bool ok=box.offer(sample,sampledAt);
   if(ok)stopped_=stopped;
   return ok;
  }
  if(tracking)return false;
  if(offered_&&!newStop&&uint32_t(now-at_)<INTERVAL_MS)return false;
  if(!box.offer(sample,sampledAt))return false;
  offered_=true;stopped_=stopped;at_=now;return true;
 }
 void suppressNonStopped(StatusOutbox& box,bool correlatedMission){
  if(correlatedMission&&box.pending()&&box.status().stopState!=1U)box.reset();
 }
};
}
