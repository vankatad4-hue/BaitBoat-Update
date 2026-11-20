#include "../source/carrier_safety_core.h"
#include <cassert>
#include <iostream>
using namespace carrier;
int main(){
 uint8_t p[SIZE];Command c{OUTPUT,33,-123,100,-1},r{};
 assert(encode(c,p)&&decode(p,SIZE,r)&&r.left==-123&&r.seq==33);
 p[10]^=1;assert(!decode(p,SIZE,r));assert(!decode(p,15,r));assert(!encode({OUTPUT,1,50,0,0},p));
 SafetyCore s;const uint16_t durations[4]={60,70,80,90};Inputs in={uint8_t(STOP_OK|WATCHDOG_OK|INTERLOCK_OK|LEFT_IDLE|RIGHT_IDLE|COILS_IDLE|FEEDBACK_OK),15};
 assert(!s.configure(false,durations));assert(s.receive({SYNC_OFF,1,0,0,-1},0,in));assert(!s.tick(0,in).permit);
 assert(s.configure(true,durations));assert(s.receive({SYNC_OFF,1,0,0,-1},0,in));auto d=s.tick(0,in);assert(d.permit&&d.pet&&!d.left);
 in.flags|=GATE_ON;s.tick(1,in);assert(s.status(1,in).state==ARMED);
 assert(s.receive({OUTPUT,2,200,200,-1},2,in));assert(s.tick(2,in).left==200);
 assert(!s.receive({OUTPUT,2,200,200,-1},3,in));assert(!s.tick(3,in).permit);
 in.flags&=~GATE_ON;assert(s.receive({SYNC_OFF,10,0,0,-1},10,in));s.tick(10,in);in.flags|=GATE_ON;s.tick(11,in);
 assert(s.receive({OUTPUT,11,0,0,1},12,in));assert(s.tick(12,in).coil==1);
 assert(s.receive({OUTPUT,12,0,0,1},31,in));assert(s.receive({OUTPUT,13,0,0,1},50,in));assert(s.tick(71,in).coil==1);
 assert(!s.tick(82,in).permit); // independent pulse cap, despite refreshed lease
 in.flags&=~GATE_ON;assert(s.receive({SYNC_OFF,20,0,0,-1},100,in));s.tick(100,in);in.flags|=GATE_ON;s.tick(101,in);
 assert(s.receive({OUTPUT,21,0,0,-1},102,in));assert(!s.tick(142,in).permit);
 assert(!s.receive({SYNC_OFF,22,0,0,-1},143,in));in.flags&=~GATE_ON;
 assert(s.receive({SYNC_OFF,23,0,0,-1},150,in));s.tick(150,in);in.flags|=GATE_ON;s.tick(151,in);
 assert(s.receive({OUTPUT,24,0,0,0},152,in));assert(!s.receive({OUTPUT,25,0,0,1},153,in));
 auto a=s.status(17,in);encodeStatus(a,p);Status b{};assert(decodeStatus(p,SIZE,b)&&b.sample==17&&b.state==FAULT);
 std::cout<<"Carrier CRC, sequence, arm, lease and independent pulse limits passed\n";
}
