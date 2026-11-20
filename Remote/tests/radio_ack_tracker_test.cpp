#include "radio_ack_tracker.h"
#include <assert.h>
static RadioDecodedPayload ack(uint16_t id,uint8_t type,uint8_t result) {
    RadioDecodedPayload p{}; p.kind=RADIO_PAYLOAD_ACK; p.commandId=id; p.commandType=type; p.ackResult=result;
    p.stopState=2; return p;
}
int main(){
    RadioAckTracker t; t.reset(17);
    assert(t.begin(101,1,0,100,500) && t.pendingCount()==1);
    assert(!t.observe(ack(101,1,RADIO_ACK_EXECUTED),RADIO_ACK_LINK_PRIMARY,16,110));
    assert(t.observe(ack(101,1,RADIO_ACK_ACCEPTED),RADIO_ACK_LINK_PRIMARY,17,120));
    RadioCommandAckRecord r{}; assert(t.get(101,r) && r.state==RADIO_CMD_ACK_ACCEPTED);
    assert(t.observe(ack(101,1,RADIO_ACK_EXECUTED),RADIO_ACK_LINK_PRIMARY,17,140));
    assert(t.get(101,r) && r.state==RADIO_CMD_ACK_EXECUTED && t.pendingCount()==0);
    assert(t.begin(102,7,0,200,1000));
    assert(!t.observe(ack(102,7,RADIO_ACK_REJECTED_SAFETY),RADIO_ACK_LINK_BACKUP,17,220));
    assert(t.observe(ack(102,7,RADIO_ACK_REJECTED_SAFETY),RADIO_ACK_LINK_PRIMARY,17,230));
    assert(t.get(102,r) && r.state==RADIO_CMD_ACK_REJECTED);
    assert(t.begin(103,3,2,300,1000)); // RETURN HOME is backup-eligible
    assert(t.observe(ack(103,3,RADIO_ACK_ACCEPTED),RADIO_ACK_LINK_BACKUP,17,320));
    t.update(1300); assert(t.get(103,r) && r.state==RADIO_CMD_ACK_TIMEOUT);
    assert(t.begin(104,1,0,1400,100)); // STOP may be acknowledged on backup
    assert(t.observe(ack(104,1,RADIO_ACK_EXECUTED),RADIO_ACK_LINK_BACKUP,17,1450));
    assert(t.get(104,r) && r.state==RADIO_CMD_ACK_EXECUTED);
    assert(t.begin(105,4,0,0xFFFFFFF0UL,40)); t.update(0x00000018UL);
    assert(t.get(105,r) && r.state==RADIO_CMD_ACK_TIMEOUT); // millis rollover
    assert(!t.begin(0,1,0,1) && !t.begin(106,11,0,1));
    assert(t.begin(107,4,0,2,100)); assert(!t.begin(107,4,0,3,100));
    return 0;
}
