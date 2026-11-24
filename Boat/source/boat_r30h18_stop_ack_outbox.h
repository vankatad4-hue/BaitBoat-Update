#pragma once
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
