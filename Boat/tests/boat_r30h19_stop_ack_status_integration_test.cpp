// R30H-19 full logical STOP -> Boat ACK -> primary STATUS -> Remote gate.
// Zero ESP GPIO, zero RF; the simulated TX slot can be occupied or dropped.
#include "../source/boat_r30h18_stop_ack_outbox.h"
#include "../source/boat_r30h19_status_outbox.h"
#include "../../Remote/source/r30h17_stop_confirmation_gate.h"
#include "../../Remote/source/radio_ack_tracker.h"
#include <cassert>
#include <iostream>
using boat_r30h18::StopAckOutbox;
using boat_r30h19::StatusOutbox;
using flytec_r30h17::StopConfirmationGate;
static RadioDecodedPayload status(uint8_t stopped){
    // Real shared protocol payload, then Remote-compatible decoding.
    uint8_t raw[8]{};
    assert(radio_encodeTelemetryStatusPayload(0,stopped,0,255,0,0,raw));
    RadioDecodedPayload decoded{};
    assert(radio_decodeTelemetryPayload(raw,decoded));
    assert(decoded.kind==RADIO_PAYLOAD_STATUS&&decoded.stopState==stopped);
    return decoded;
}
// Outer 32-byte RadioFrame HMAC/epoch/sequence validation is performed by
// production dispatch before this test's ACK/STATUS decoder invocation.
static bool remoteAck(RadioAckTracker &tracker,StopConfirmationGate &gate,
                      uint16_t id,uint8_t epoch,uint32_t now){
    uint8_t raw[8]{};
    assert(radio_encodeAckPayload(RADIO_ACK_ACCEPTED,1,id,0,0,raw));
    RadioDecodedPayload decoded{};
    assert(radio_decodeAckPayload(raw,decoded));
    if(!tracker.observe(decoded,RADIO_ACK_LINK_PRIMARY,epoch,now))return false;
    return gate.acceptedAck(decoded.commandId,
                            decoded.ackResult==RADIO_ACK_ACCEPTED);
}
int main(){
    StopAckOutbox ack; StatusOutbox post; StopConfirmationGate remote;
    RadioAckTracker tracker;tracker.reset(1);
    assert(tracker.begin(91,1,0,10,2500));
    remote.request(91);
    // A STATUS created before STOP is invalidated on receipt of new STOP.
    assert(post.offer(status(1),10));
    post.reset();
    assert(ack.offer(91,1,0,0,0,20));
    // The radio slot is occupied by an unrelated earlier packet.
    assert(!ack.due(21,true,true,true));
    assert(!remote.allowsStopStatus(true,true));
    // New actual physical-sample STATUS appears while ACK still pending.
    assert(post.offer(status(1),30));
    assert(!post.due(31,true,true,false,ack.pending()));
    // Slot frees, first STOP ACK radio TX fails.
    assert(ack.due(40,true,true,false)); ack.admitted(40);
    assert(!post.due(41,true,true,true,ack.pending()));
    ack.completed(false);
    assert(!remote.allowsStopStatus(true,true));
    // Recovery after 75ms: successful RF ACK transmitted, but application
    // packet is dropped before Remote sees it. No confirmation is possible.
    assert(ack.due(115,true,true,false));ack.admitted(115);
    ack.completed(true);assert(!ack.pending());
    assert(post.due(116,true,true,false,ack.pending()));
    post.sent(); // Dropped before Remote.
    assert(!remote.allowsStopStatus(true,true));
    // Remote resends same command-ID STOP; Boat accepts it again.
    assert(ack.offer(91,1,0,0,0,180));
    assert(ack.due(181,true,true,false));ack.admitted(181);ack.completed(true);
    assert(!remoteAck(tracker,remote,90,1,182)); // wrong command ID
    assert(!remoteAck(tracker,remote,91,2,182)); // wrong session epoch
    assert(remoteAck(tracker,remote,91,1,183));
    // Earlier ACK must not retroactively authorize an old or unknown status.
    assert(!remote.allowsStopStatus(true,false));
    assert(!remote.allowsStopStatus(false,true));
    assert(post.offer(status(2),182));
    assert(post.due(190,true,true,false,ack.pending()));
    assert(!remote.allowsStopStatus(true,post.status().stopState==1));
    post.sent();
    // Freshly observed stopped-output status gets its own primary TX slot.
    assert(post.offer(status(1),220));
    assert(post.due(221,true,true,false,ack.pending()));
    assert(remote.allowsStopStatus(true,post.status().stopState==1));
    post.sent();
    // Superseding STOP requires a new correlated ACK and new status.
    remote.request(92);
    assert(tracker.begin(92,1,0,225,2500));
    assert(!remote.allowsStopStatus(true,true));
    assert(!remoteAck(tracker,remote,91,1,226));
    assert(ack.offer(92,1,0,0,0,230));
    assert(post.offer(status(1),231));
    assert(!post.due(232,true,true,false,ack.pending()));
    assert(ack.due(233,true,true,false));ack.admitted(233);ack.completed(true);
    assert(remoteAck(tracker,remote,92,1,234));
    assert(post.due(234,true,true,false,ack.pending()));
    assert(remote.allowsStopStatus(true,true));
    // Reset, stale sample, busy nRF or transport loss never confirm a STOP.
    post.reset();remote.request(93);
    assert(post.offer(status(1),500));
    assert(!post.due(501,true,true,true,false));
    assert(!post.due(801,true,true,false,false));assert(!post.pending());
    assert(!remote.allowsStopStatus(true,true));
    assert(post.offer(status(1),900));
    assert(!post.due(901,false,true,false,false));assert(!post.pending());
    assert(post.offer(status(1),902));
    assert(!post.due(903,true,false,false,false));assert(!post.pending());
    assert(!post.offer(RadioDecodedPayload{},999));
    assert(!post.pending());
    // 32-bit millis wrap is safe.
    assert(post.offer(status(1),0xfffffff0U));
    assert(post.due(0x00000010U,true,true,false,false));
    assert(!post.due(0x00000120U,true,true,false,false));
    std::cout<<"R30H-19 STOP ACK + post-ACK primary STATUS fault matrix PASS (host only)\n";
}
