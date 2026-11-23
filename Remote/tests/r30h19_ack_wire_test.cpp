// R30H-19: real Remote ACK payload decoder + tracker + correlation gate.
// Assumes outer RadioFrame HMAC/session/sequence acceptance by production
// validateInboundFrame(); tests NO radio, GPIO, timing measurements or motors.
#include "../source/radio_payload_codec.h"
#include "../source/radio_ack_tracker.h"
#include "../source/r30h17_stop_confirmation_gate.h"
#include <cassert>
#include <iostream>
static RadioDecodedPayload ackPayload(uint16_t id,uint8_t type,uint8_t result){
 uint8_t b[8]{};
 assert(radio_encodeAckPayload(result,type,id,0,0,b));
 RadioDecodedPayload decoded{};
 assert(radio_decodeAckPayload(b,decoded));
 assert(decoded.kind==RADIO_PAYLOAD_ACK&&decoded.commandId==id);
 return decoded;
}
int main(){
 RadioAckTracker tracker; tracker.reset(1);
 flytec_r30h17::StopConfirmationGate gate;
 assert(tracker.begin(91,1,0,100,2500));gate.request(91);
 assert(!tracker.observe(ackPayload(90,1,RADIO_ACK_ACCEPTED),
                         RADIO_ACK_LINK_PRIMARY,1,105));
 assert(!tracker.observe(ackPayload(91,2,RADIO_ACK_ACCEPTED),
                         RADIO_ACK_LINK_PRIMARY,1,105));
 assert(!tracker.observe(ackPayload(91,1,RADIO_ACK_ACCEPTED),
                         RADIO_ACK_LINK_PRIMARY,2,105));
 assert(!gate.allowsStopStatus(true,true));
 auto first=ackPayload(91,1,RADIO_ACK_ACCEPTED);
 assert(tracker.observe(first,RADIO_ACK_LINK_PRIMARY,1,106));
 assert(gate.acceptedAck(first.commandId,true));
 assert(!gate.allowsStopStatus(false,true)); // backup status not enough
 assert(!gate.allowsStopStatus(true,false));
 uint8_t wire[8]{};
 assert(radio_encodeTelemetryStatusPayload(0,2,0,255,0,0,wire));
 RadioDecodedPayload observed{};
 assert(radio_decodeTelemetryPayload(wire,observed));
 assert(!gate.allowsStopStatus(true,observed.stopState==1));
 assert(radio_encodeTelemetryStatusPayload(0,1,0,255,0,0,wire));
 assert(radio_decodeTelemetryPayload(wire,observed));
 assert(gate.allowsStopStatus(true,observed.stopState==1));
 // A new STOP invalidates the old ACK/STATUS evidence.
 gate.request(92);
 assert(tracker.begin(92,1,0,150,2500));
 assert(!gate.allowsStopStatus(true,true));
 assert(!gate.acceptedAck(91,true));
 const auto denied=ackPayload(92,1,RADIO_ACK_REJECTED_SAFETY);
 assert(tracker.observe(denied,RADIO_ACK_LINK_PRIMARY,1,155));
 assert(!gate.acceptedAck(denied.commandId,false));
 assert(!gate.allowsStopStatus(true,true));
 // Once rejected, a forged later ACCEPTED cannot reuse the closed record.
 assert(!tracker.observe(ackPayload(92,1,RADIO_ACK_ACCEPTED),
                         RADIO_ACK_LINK_PRIMARY,1,156));
 // Fresh command-ID can be separately confirmed on valid primary.
 gate.request(93);
 assert(tracker.begin(93,1,0,200,2500));
 const auto accepted=ackPayload(93,1,RADIO_ACK_ACCEPTED);
 assert(tracker.observe(accepted,RADIO_ACK_LINK_PRIMARY,1,201));
 assert(gate.acceptedAck(accepted.commandId,true));
 assert(gate.allowsStopStatus(true,true));
 // Session reset revokes all pending and accepted evidence.
 tracker.reset(2);gate.reset();gate.request(94);
 assert(tracker.begin(94,1,0,220,2500));
 assert(!tracker.observe(ackPayload(94,1,RADIO_ACK_ACCEPTED),
                         RADIO_ACK_LINK_PRIMARY,1,221));
 assert(!gate.allowsStopStatus(true,true));
 std::cout<<"R30H-19 Remote STOP ACK payload/tracker/STATUS gate PASS (host only)\n";
}
