#pragma once
// Consolidated STOP delivery policy; original types, namespaces and limits retained.
// R30H-16: bounded, STOP-only retry until authenticated Boat STOP status.
// Pure in-memory admission policy; no RF/ESP/GPIO/actuators/clock calls.
// A successful radio TX-buffer write is never treated as Boat STOP execution.
// A session reset, transport fault or age expiry prevents stale replay.
// Logical command ID remains stable; radio transport gives fresh sequences.
#include <stdint.h>
namespace flytec_r30h15 {
template <class Command>
class PrimaryStopRetry final {
public:
    static constexpr uint32_t MAX_QUEUE_AGE_MS=2500U;
    static constexpr uint32_t RETRANSMIT_GAP_MS=150U;
    static constexpr uint8_t MAX_TRANSMISSIONS=8U;
    void reset(){
        pending_=false;queued_=Command{};queuedAtMs_=lastAdmissionAtMs_=0U;
        transmissions_=0U;
    }
    void queue(const Command&command,uint32_t now){
        queued_=command;queuedAtMs_=now;
        lastAdmissionAtMs_=0U;transmissions_=0U;pending_=true;
    }
    bool pending()const{return pending_;}
    uint32_t queuedAtMs()const{return queuedAtMs_;}
    uint32_t commandId()const{return pending_?queued_.commandId:0U;}
    uint8_t transmissions()const{return transmissions_;}
    void noteAdmitted(uint32_t now){
        if(pending_&&transmissions_<MAX_TRANSMISSIONS){
            lastAdmissionAtMs_=now;++transmissions_;
        }
    }
    bool expired(uint32_t now)const{
        return pending_&&(uint32_t)(now-queuedAtMs_)>MAX_QUEUE_AGE_MS;
    }
    bool mayAttempt(bool sessionReady,bool transportReady,bool txBusy,uint32_t now)const{
        return pending_&&sessionReady&&transportReady&&!txBusy&&!expired(now)&&
               transmissions_<MAX_TRANSMISSIONS&&
               (transmissions_==0U||
                (uint32_t)(now-lastAdmissionAtMs_)>=RETRANSMIT_GAP_MS);
    }
    bool maySendPeriodic(bool transportReady,bool txBusy)const{
        // Until the STOP retry budget is exhausted, never let heartbeat
        // steal its TX slot. Afterwards allow heartbeat so Boat can keep
        // reporting authoritative STOP feedback while confirmation waits.
        return transportReady&&!txBusy&&
               (!pending_||transmissions_>=MAX_TRANSMISSIONS);
    }
    const Command&command()const{return queued_;}
private:
    Command queued_{};
    uint32_t queuedAtMs_=0U,lastAdmissionAtMs_=0U;
    uint8_t transmissions_=0U;
    bool pending_=false;
};
} // namespace flytec_r30h15

// R30H-17: STOP request/ACK/feedback correlation; pure state, no hardware.
// Only authenticated/sequence-checked, command-ID-matching ACCEPTED/EXECUTED
// evidence may unlock the subsequent primary Boat STATUS confirmation.
// A bare STATUS=STOP after a locally requested STOP is insufficient: it may
// have been produced before the command was accepted. Callers must separately
// verify HMAC/session/sequence, link identity and local feedback freshness.
#include <stdint.h>
namespace flytec_r30h17 {
class StopConfirmationGate final {
public:
    void reset(){armed_=false;ackAccepted_=false;id_=0;}
    void request(uint32_t commandId){
        armed_=true;ackAccepted_=false;id_=(uint16_t)commandId;
    }
    bool armed() const{return armed_;}
    uint16_t commandId()const{return id_;}
    bool ackAccepted()const{return ackAccepted_;}
    // Call only after the existing ACK tracker has accepted a matching
    // authenticated, session-scoped ACK of STOP (not a different command).
    bool acceptedAck(uint16_t commandId,bool accepted){
        if(!armed_||!id_||commandId!=id_||!accepted)return false;
        ackAccepted_=true;return true;
    }
    bool allowsStopStatus(bool primary,bool stopped)const{
        return primary&&stopped&&(!armed_||ackAccepted_);
    }
private:
    uint16_t id_=0;
    bool armed_=false,ackAccepted_=false;
};
} // namespace flytec_r30h17
