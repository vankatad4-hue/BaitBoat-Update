#pragma once
#include <Arduino.h>
#include <Stream.h>
#include "radio_protocol.h"

typedef void (*E22SetModeFn)(bool m0,bool m1,void *context);
typedef bool (*E22AuxReadyFn)(void *context);
typedef void (*E22ResetFn)(bool released,void *context);

class E22UartLink {
public:
    E22UartLink(Stream &uart,E22SetModeFn modeFn,E22AuxReadyFn auxFn,E22ResetFn resetFn,void *context);
    bool configureEU868TestProfile(); // blocking setup-only readback/config
    void update();                    // non-blocking RX/TX-state poll
    bool sendFrame(const RadioFrame &frame,uint32_t nowMs);
    bool receiveFrame(RadioFrame &frame);
    bool ready() const { return configured_ && auxReadyNow_; }
    uint32_t estimatedAirtimeMs() const { return EST_AIRTIME_MS; }
    uint32_t txCount() const { return txCount_; }
    uint32_t rxCount() const { return rxCount_; }
    uint32_t rejectedDutyCount() const { return rejectedDutyCount_; }
private:
    static constexpr uint32_t EST_AIRTIME_MS=25; // conservative until measured
    static constexpr uint32_t MAX_AIRTIME_PER_HOUR_MS=36000; // 1% band duty budget
    static constexpr uint8_t SUBPACKET_BYTES=32;
    Stream &uart_;
    E22SetModeFn modeFn_;
    E22AuxReadyFn auxFn_;
    E22ResetFn resetFn_;
    void *context_;
    bool configured_;
    bool auxReadyNow_;
    bool txPending_;
    uint32_t txStartedMs_;
    uint8_t rxBuffer_[RADIO_FRAME_SIZE];
    uint8_t rxIndex_;
    RadioFrame readyFrame_;
    bool frameAvailable_;
    struct AirtimeBucket { uint32_t minuteStamp; uint16_t airtimeMs; bool valid; };
    AirtimeBucket buckets_[60];
    uint32_t txCount_,rxCount_,rejectedDutyCount_;
    void clearBuckets();
    uint32_t usedAirtimeMs(uint32_t nowMs);
    bool dutyAllows(uint32_t nowMs);
    void recordAirtime(uint32_t nowMs,uint32_t airtimeMs);
    bool configCommand(const uint8_t *command,size_t length,const uint8_t *expected,size_t expectedLength,uint32_t timeoutMs);
    void parserByte(uint8_t value);
};
