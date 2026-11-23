#pragma once
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
