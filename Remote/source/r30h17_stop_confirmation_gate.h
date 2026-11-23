#pragma once
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
