// R30H-20: both actual 32-byte Boat and Remote RadioFrame implementations
// compiled separately against HOST ONLY OpenSSL-backed mbedTLS shim.
// HMAC/CRC/epoch/ID/link-direction/replay/payload/STOP gate simulated.
// No RF GPIO, no actuator and no physically measured latency.
#include "radio_protocol.h"
#include "radio_payload_codec.h"
#include "radio_session_v2.h"
#include "../Remote/source/r30h17_stop_confirmation_gate.h"
#include <openssl/hmac.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <cstddef>
static bool sha(const uint8_t k[32],const uint8_t*d,size_t n,uint8_t o[32]){
    unsigned written=0;
    return HMAC(EVP_sha256(),k,32,d,n,o,&written)!=nullptr&&written==32;
}
static void derive(const uint8_t root[32],uint64_t boatNonce,uint8_t out[32]){
    const RadioSessionTranscript transcript{101U,boatNonce,1,2};
    RadioSessionKeys keys{};
    assert(radioSession_derive(root,transcript,sha,keys));
    std::memcpy(out,keys.frameKey,32);
    radioSession_wipe(&keys,sizeof(keys));
}
static RadioFrame ack(uint16_t seq,uint16_t id,uint8_t epoch,
                      const uint8_t key[32],bool backup=false){
    RadioFrame f{};
    radio_frameInit(f,RADIO_MSG_ACK,2,1,seq,id,12000U+seq,epoch);
    f.flags=backup?RADIO_FLAG_BACKUP_LINK:0;
    f.payload[0]=0;f.payload[1]=0; // ACCEPTED
    f.payload[2]=1; // EMERGENCY_STOP
    f.payload[3]=(uint8_t)id;f.payload[4]=(uint8_t)(id>>8);
    f.payload[5]=2; // physical STOP status unknown in ACK
    assert(radio_frameSeal(f,key));
    return f;
}
static RadioFrame status(uint16_t seq,uint8_t stop,uint8_t epoch,
                         const uint8_t key[32],bool backup=false){
    RadioFrame f{};
    radio_frameInit(f,RADIO_MSG_TELEMETRY,2,1,seq,0,12000U+seq,epoch);
    f.flags=backup?RADIO_FLAG_BACKUP_LINK:0;
    f.payload[0]=1;f.payload[1]=0;f.payload[2]=stop;
    f.payload[3]=0;f.payload[4]=75;f.payload[5]=3;f.payload[6]=10;
    assert(radio_frameSeal(f,key));
    return f;
}
struct Receiver {
    uint8_t epoch;
    uint8_t key[32]{};
    RadioSessionCounter rx{};
    flytec_r30h17::StopConfirmationGate gate{};
    bool confirmed=false;
    Receiver(uint8_t e,const uint8_t k[32]):epoch(e){std::memcpy(key,k,32);}
    void request(uint16_t id){gate.request(id);confirmed=false;}
    // Production receive() validates source/destination/link/epoch and
    // HMAC+sequence BEFORE this portable payload evidence gate.
    bool receive(const RadioFrame &f,bool onPrimary=true){
        if(!radio_frameVerify(f,key)||f.sourceId!=2||f.destinationId!=1||
           f.sessionEpoch!=epoch||!radioLinkFlagMatches(f.flags,!onPrimary)||
           !rx.accept(f.sequence))return false;
        RadioDecodedPayload parsed{};
        if(f.type==RADIO_MSG_ACK){
            if(!radio_decodeAckPayload(f.payload,parsed)||parsed.commandId!=f.commandId||
               parsed.commandType!=1||!onPrimary)return false;
            return gate.acceptedAck(parsed.commandId,
                    parsed.ackResult==RADIO_ACK_ACCEPTED||
                    parsed.ackResult==RADIO_ACK_EXECUTED);
        }
        if(f.type==RADIO_MSG_TELEMETRY){
            if(!radio_decodeTelemetryPayload(f.payload,parsed)||
               parsed.kind!=RADIO_PAYLOAD_STATUS)return false;
            if(gate.allowsStopStatus(onPrimary,parsed.stopState==1))confirmed=true;
            return true;
        }
        return false;
    }
};
int main(){
    static_assert(sizeof(RadioFrame)==32,"32-byte RF24/LoRa wire contract");
    static_assert(offsetof(RadioFrame,authTag)==24,"8-byte HMAC tag end");
    uint8_t root[32]{};for(unsigned i=0;i<32;++i)root[i]=(uint8_t)(i+1);
    uint8_t key[32]{};derive(root,211U,key);
    Receiver remote(7,key);
    remote.request(900);
    // STOP status before a matched ACK cannot clear the local STOP latch.
    assert(remote.receive(status(1,1,7,key))&&!remote.confirmed);
    const auto lostAck=ack(2,900,7,key);
    assert(radio_frameVerify(lostAck,key)); // on-air drop: do not call receive
    assert(remote.receive(status(3,1,7,key))&&!remote.confirmed);
    // STOP retries reuse command ID, but each RF frame gets a fresh sequence.
    const auto accepted=ack(4,900,7,key);
    assert(remote.receive(accepted));
    assert(!remote.receive(accepted)); // replay
    assert(remote.receive(status(5,2,7,key))&&!remote.confirmed); // UNKNOWN
    assert(remote.receive(status(6,1,7,key))&&remote.confirmed);
    // New STOP cannot reuse an old ACK or earlier STOP feedback.
    remote.request(901);
    assert(!remote.receive(ack(4,900,7,key)));
    assert(remote.receive(status(7,1,7,key))&&!remote.confirmed);
    assert(!remote.receive(ack(8,900,7,key))&&!remote.confirmed);
    assert(remote.receive(ack(9,901,7,key)));
    assert(remote.receive(status(10,1,7,key))&&remote.confirmed);
    // A valid authentication tag is necessary; CRC alone cannot forge it.
    remote.request(902);
    auto corrupted=ack(11,902,7,key);corrupted.payload[3]^=1U;
    assert(!radio_frameVerify(corrupted,key));
    corrupted.crc16=radio_crc16((const uint8_t*)&corrupted,
                               offsetof(RadioFrame,crc16));
    assert(!radio_frameVerify(corrupted,key));
    assert(!remote.receive(corrupted)&&!remote.confirmed);
    // Backups are authenticated but must not masquerade as primary ACK/STATUS.
    assert(!remote.receive(ack(12,902,7,key,true),true));
    assert(!remote.receive(status(13,1,7,key,true),true));
    assert(!remote.confirmed);
    // Out-of-order valid STATUS (higher seq) invalidates an earlier ACK.
    assert(remote.receive(status(20,1,7,key))&&!remote.confirmed);
    assert(!remote.receive(ack(19,902,7,key)));
    assert(remote.receive(ack(21,902,7,key)));
    assert(remote.receive(status(22,1,7,key))&&remote.confirmed);
    // Incorrect endpoint IDs and epoch are never session evidence.
    remote.request(903);
    auto misaddressed=ack(23,903,7,key);
    misaddressed.destinationId=3;assert(radio_frameSeal(misaddressed,key));
    assert(!remote.receive(misaddressed));
    assert(!remote.receive(ack(24,903,8,key)));
    assert(!remote.confirmed);
    // New session derives a NEW key; old session's frames cannot replay.
    uint8_t freshKey[32]{};derive(root,212U,freshKey);
    Receiver next(8,freshKey);next.request(903);
    assert(!next.receive(ack(1,903,7,key)));
    assert(!next.receive(ack(1,903,8,key))); // old key, new epoch
    assert(next.receive(ack(1,903,8,freshKey)));
    assert(next.receive(status(2,1,8,freshKey))&&next.confirmed);
    std::cout<<"R30H-20 signed 32-byte STOP ACK/status replay/loss/rekey matrix PASS (host only)\n";
}
