#pragma once
#include <Arduino.h>
#include <Stream.h>
#include "radio_protocol.h"
#include "e22_tx_state.h"
#include "lora_link_policy.h"

typedef void (*E22SetModeFn)(bool m0,bool m1,void *context);
typedef bool (*E22AuxReadyFn)(void *context);
typedef size_t (*E22TxCapacityFn)(void *context);
typedef void (*E22ResetFn)(bool released,void *context);

class E22UartLink {
public:
    E22UartLink(Stream &uart,E22SetModeFn modeFn,E22AuxReadyFn auxFn,E22ResetFn resetFn,void *context,E22TxCapacityFn capacityFn=nullptr);
    bool configureEU868TestProfile(); // blocking setup-only readback/config
    // Owner only: revoke admission and mark outstanding UART TX uncertain.
    void shutdownLocal(){disableAdmission();if(txState_.pending())txState_.uncertain();}
    void disableAdmission(){configured_=false;rxIndex_=rxHead_=rxTail_=rxQueueCount_=0;} // owner only; does not stop RF
    void update();                    // non-blocking RX/TX-state poll
    bool sendFrame(const RadioFrame &frame,uint32_t nowMs);
    enum class Admission:uint8_t { Accepted, Busy, DutyLimited, Invalid, Unavailable, Uncertain };
    Admission trySendFrame(const RadioFrame &frame,uint32_t nowMs); // no implicit poll; Stream write may block
    bool receiveFrame(RadioFrame &frame); // pure software dequeue
    bool txPending()const{return txState_.pending();}
    flytec_rtos::E22TxState::Outcome txOutcome()const{return txState_.outcome();}
    bool configured()const{return configured_;}
    bool transportFault()const{return txState_.fault();}
    bool ready() const { return configured_ && auxReadyNow_ && !txState_.pending() && !txState_.fault(); }
    uint32_t estimatedAirtimeMs() const { return EST_AIRTIME_MS; }
    uint32_t txCount() const { return txCount_; }
    uint32_t rxCount() const { return rxCount_; }
    uint32_t rejectedDutyCount() const { return rejectedDutyCount_; }
    // Owner-task diagnostic only: full queue observations, not lost frames.
    uint32_t rxFullPollCount() const { return rxFullPollCount_; }
private:
    uint32_t rxFullPollCount_=0;
    static constexpr uint32_t EST_AIRTIME_MS=flytec_rtos::LoRaAirtimeProfile::BaselinePacket; // UNVERIFIED estimate; RF TX locked until actual airtime is measured
    static constexpr uint32_t MAX_AIRTIME_PER_HOUR_MS=flytec_rtos::LoRaAirtimeProfile::BaselineHour; // PROVISIONAL 1% policy; actual EU band conditions need signoff
    static constexpr uint8_t SUBPACKET_BYTES=32;
    Stream &uart_;
    E22SetModeFn modeFn_;
    E22AuxReadyFn auxFn_;
    E22ResetFn resetFn_;
    void *context_;
    E22TxCapacityFn capacityFn_;
    bool configured_;
    bool auxReadyNow_;
    flytec_rtos::E22TxState txState_;
    uint8_t rxBuffer_[RADIO_FRAME_SIZE];
    uint8_t rxIndex_;
    RadioFrame rxQueue_[4];
    uint8_t rxHead_,rxTail_,rxQueueCount_;
    // Legacy accounting uses the same rolling implementation as the
    // adaptive policy. It remains provisional until measured RF-tail and
    // cold-start isolation are bound to one exclusive physical owner.
    flytec_rtos::LoRaRollingLedger airtime_;
    uint32_t txCount_,rxCount_,rejectedDutyCount_;
    bool configCommand(const uint8_t *command,size_t length,const uint8_t *expected,size_t expectedLength,uint32_t timeoutMs);
    void parserByte(uint8_t value);
};
