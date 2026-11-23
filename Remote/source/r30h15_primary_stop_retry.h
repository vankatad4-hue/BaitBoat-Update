#pragma once
// R30H-15: emergency STOP admission retry, pure in-memory policy.
// NO RF activity, GPIO, clock API, or actuator authority in this header.
// Once a STOP is requested locally, a busy primary TX cannot silently lose it.
// Retries preserve logical command ID but use a new authenticated frame sequence.
// A session reset/transport fault clears all pending intent: never replay
// unauthenticated/stale commands in a new session.
#include <stdint.h>
namespace flytec_r30h15 {
template <class Command>
class PrimaryStopRetry final {
public:
    static constexpr uint32_t MAX_QUEUE_AGE_MS=2500U;
    void reset(){pending_=false;queued_=Command{};queuedAtMs_=0;}
    void queue(const Command&command,uint32_t now){
        queued_=command;queuedAtMs_=now;pending_=true;
    }
    bool pending()const{return pending_;}
    uint32_t queuedAtMs()const{return queuedAtMs_;}
    uint32_t commandId()const{return pending_?queued_.commandId:0U;}
    bool expired(uint32_t now)const{
        return pending_&&(uint32_t)(now-queuedAtMs_)>MAX_QUEUE_AGE_MS;
    }
    bool mayAttempt(bool sessionReady,bool transportReady,bool txBusy,uint32_t now)const{
        return pending_&&sessionReady&&transportReady&&!txBusy&&!expired(now);
    }
    bool maySendPeriodic(bool transportReady,bool txBusy)const{
        return transportReady&&!txBusy&&!pending_;
    }
    const Command&command()const{return queued_;}
private:
    Command queued_{};
    uint32_t queuedAtMs_=0;
    bool pending_=false;
};
} // namespace flytec_r30h15
