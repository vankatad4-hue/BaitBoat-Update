#include "boat_handshake_reply.h"
#include "boat_radio_safety.h"
#include <cassert>
int main(){
 BoatHandshakeReply p,b;
 RadioFrame f{};f.type=RADIO_MSG_SESSION_CHALLENGE;f.sourceId=2;f.destinationId=1;f.payload[0]=7;
 assert(p.offer(f,100,false));assert(!b.offer(f,100,true));
 f.flags=RADIO_FLAG_BACKUP_LINK;assert(b.offer(f,100,true));
 unsigned attempts=0;
 auto busy=[&](const RadioFrame&){++attempts;return false;};
 auto admit=[&](const RadioFrame& r){++attempts;assert(r.flags==RADIO_FLAG_BACKUP_LINK);return true;};
 assert(!b.service(100,6000,250,false,false,busy)&&attempts==0);
 assert(!b.service(101,6000,250,true,true,busy)&&attempts==0);
 assert(!b.service(102,6000,250,true,false,busy)&&attempts==1&&b.pending());
 assert(!b.service(351,6000,250,true,false,admit)&&attempts==1);
 assert(b.offer(f,352,true)); // duplicate does not replace or extend
 assert(b.service(352,6000,250,true,false,admit)&&attempts==2&&!b.pending());
 assert(p.pending());p.expire(3600,3500);assert(!p.pending());
 assert(b.offer(f,1000,true));assert(b.offer(f,6999,true));
 auto confirm=f;confirm.type=RADIO_MSG_SESSION_CONFIRM;
 assert(!b.offer(confirm,6999,true));
 assert(!b.service(7000,6000,250,true,false,admit)&&!b.pending());
 assert(b.offer(f,UINT32_MAX-10,true));b.reset();assert(!b.pending());
 assert(b.offer(confirm,UINT32_MAX-10,true));
 assert(b.service(10,6000,250,true,false,admit));
 // RTH uses last authenticated application RX, not handshake traffic.
 const uint32_t last=1000;
 assert(!boatRadioLossRthDue(true,true,true,true,true,last,10999));
 assert(boatRadioLossRthDue(true,true,true,true,true,last,11000));
 assert(!boatRadioLossRthDue(false,true,true,true,true,last,11000));
 assert(!boatRadioLossRthDue(true,true,true,false,true,last,11000));
}
