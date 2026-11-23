// R30H-17 STOP evidence gate: no RF, GPIO or actuator.
#include "r30h17_stop_confirmation_gate.h"
#include <assert.h>
int main(){
    flytec_r30h17::StopConfirmationGate gate;
    assert(!gate.armed());
    assert(gate.allowsStopStatus(true,true)); // safe initial boot handshake
    assert(!gate.allowsStopStatus(false,true)); // backup-only cannot clear
    assert(!gate.allowsStopStatus(true,false));
    gate.request(124U);
    assert(gate.armed() && gate.commandId()==124U);
    assert(!gate.allowsStopStatus(true,true)); // stale STOP from before request
    assert(!gate.acceptedAck(123U,true)); // old STOP
    assert(!gate.acceptedAck(124U,false)); // BUSY/REJECTED is not accepted
    assert(!gate.allowsStopStatus(true,true));
    assert(gate.acceptedAck(124U,true)); // authenticated matched ACK seen
    assert(gate.ackAccepted());
    assert(!gate.allowsStopStatus(false,true));
    assert(!gate.allowsStopStatus(true,false));
    assert(gate.allowsStopStatus(true,true)); // later sequenced primary status
    gate.request(125U); // new STOP revokes prior evidence
    assert(!gate.ackAccepted());
    assert(!gate.allowsStopStatus(true,true));
    assert(!gate.acceptedAck(124U,true));
    gate.reset(); // rekey/session loss revokes pending request evidence
    assert(!gate.armed() && !gate.ackAccepted());
    gate.request(0x12345678U); // over-the-air ID is 16-bit
    assert(gate.commandId()==0x5678U);
    assert(!gate.acceptedAck(0x1234U,true));
    assert(gate.acceptedAck(0x5678U,true));
    gate.reset();
    gate.request(0U); // reserved command ID never ACK-confirmable
    assert(!gate.acceptedAck(0U,true));
    assert(!gate.allowsStopStatus(true,true));
    return 0;
}
