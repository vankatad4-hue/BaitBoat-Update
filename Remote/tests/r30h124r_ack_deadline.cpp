#include "radio_ack_tracker.h"
#include <cassert>
int main(){
 RadioAckTracker t;t.reset(1);RadioCommandAckRecord r{};RadioDecodedPayload a{};
 a.kind=RADIO_PAYLOAD_ACK;a.commandType=2;a.commandId=11;a.ackResult=RADIO_ACK_ACCEPTED;
 assert(t.begin(11,2,0,100,1500));
 assert(!t.observe(a,RADIO_ACK_LINK_PRIMARY,1,99));
 assert(!t.observe(a,RADIO_ACK_LINK_BACKUP,1,101));
 assert(t.observe(a,RADIO_ACK_LINK_PRIMARY,1,110));
 a.ackResult=RADIO_ACK_EXECUTED;
 assert(!t.observe(a,RADIO_ACK_LINK_PRIMARY,1,109));
 assert(!t.observe(a,RADIO_ACK_LINK_PRIMARY,1,1600));
 assert(t.get(11,r)&&r.state==RADIO_CMD_ACK_ACCEPTED);
 t.update(1600);assert(t.get(11,r)&&r.state==RADIO_CMD_ACK_TIMEOUT);
 assert(!t.observe(a,RADIO_ACK_LINK_PRIMARY,1,1599));
 a.commandId=12;assert(t.begin(12,2,0,100,1500));
 assert(t.observe(a,RADIO_ACK_LINK_PRIMARY,1,1599));
 assert(t.get(12,r)&&r.state==RADIO_CMD_ACK_EXECUTED);
 a.commandId=13;assert(t.begin(13,2,0,UINT32_MAX-5,10));
 assert(!t.observe(a,RADIO_ACK_LINK_PRIMARY,1,UINT32_MAX-6));
 assert(!t.observe(a,RADIO_ACK_LINK_PRIMARY,1,4));
 assert(t.observe(a,RADIO_ACK_LINK_PRIMARY,1,3));
 assert(t.get(13,r)&&r.state==RADIO_CMD_ACK_EXECUTED);
}
