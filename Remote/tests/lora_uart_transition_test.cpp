#include "lora_link_policy.h"
#include <cassert>
#include <cstring>
using namespace flytec_rtos;
struct Port {
 bool locked=true,config=false,wrongProfile=false,wrongAck=false;uint32_t baud=9600;uint8_t reg=0x67;
 unsigned calls=0,writes=0,failAt=0;bool allow(){return ++calls!=failAt;}
 bool lockedAndIsolated()const{return locked;}
 bool modeVerified(bool c,uint32_t limit){assert(limit==500);if(!allow())return false;config=c;return true;}
 bool baudVerified(uint32_t b,uint32_t limit){assert(limit==500);if(!allow())return false;baud=b;return true;}
 bool exchangeExact(const uint8_t* cmd,unsigned n,uint8_t* out,unsigned size,uint32_t limit){
  assert(config&&baud==9600&&limit==500);if(!allow())return false;
  if(n==3){assert(cmd[0]==0xc1&&cmd[1]==0&&cmd[2]==9&&size==12);
   const uint8_t profile[]={0xc1,0,9,0,0,0,reg,0xc3,0x12,0,0,0};std::memcpy(out,profile,12);if(wrongProfile)out[8]^=1;return true;}
  assert(n==4&&cmd[0]==0xc2&&cmd[1]==3&&cmd[2]==1&&size==4);++writes;reg=cmd[3];
  const uint8_t ack[]={0xc1,3,1,reg};std::memcpy(out,ack,4);if(wrongAck)out[3]^=1;return true;
 }
 bool receiveFixtureVerified(uint32_t b,uint32_t limit){assert(!config&&baud==b&&limit==500);return allow();}
};
int main(){
 using S=E22UartTransition::State;const E22CommissioningPermit yes{true,true,true,true,true};
 assert(sc16CommissioningDivisor(1843200,9600)==12&&sc16CommissioningDivisor(1843200,115200)==1);
 assert(!sc16CommissioningDivisor(1000000,115200)&&!sc16CommissioningDivisor(1843200,0));
 auto run=[](E22UartTransition& x,Port& p,uint32_t t){while(x.step(p,t++)){};};
 Port p;E22UartTransition x;assert(!x.begin(p,{},0)&&p.calls==0);
 assert(x.begin(p,yes,0));run(x,p,0);assert(x.state()==S::VerifiedLocked&&x.verifiedBaud()==115200&&p.writes==1&&p.locked);
 assert(!x.begin(p,yes,100));assert(x.begin(p,yes,100,true));run(x,p,100);
 assert(x.state()==S::VerifiedLocked&&x.verifiedBaud()==9600&&p.reg==0x67&&p.writes==2);
 for(unsigned fail=1;fail<=8;++fail){Port q;q.failAt=fail;E22UartTransition f;assert(f.begin(q,yes,0));run(f,q,0);
  assert(f.state()==S::FaultLocked&&!f.verifiedBaud()&&q.locked);auto calls=q.calls;assert(!f.step(q,10)&&q.calls==calls);
  assert(!f.begin(q,yes,20));q.failAt=0;assert(f.begin(q,yes,20,true));run(f,q,20);assert(f.verifiedBaud()==9600&&q.reg==0x67);}
 Port bad;bad.wrongProfile=true;E22UartTransition b;assert(b.begin(bad,yes,0));run(b,bad,0);assert(b.state()==S::FaultLocked&&bad.writes==0);
 Port ack;ack.wrongAck=true;E22UartTransition a;assert(a.begin(ack,yes,0));run(a,ack,0);assert(a.state()==S::FaultLocked&&ack.writes==1);
 ack.wrongAck=false;assert(a.begin(ack,yes,100,true));run(a,ack,100);assert(a.verifiedBaud()==9600);
 Port timeout;E22UartTransition t;assert(t.begin(timeout,yes,0xffffff00U));assert(!t.step(timeout,0xffffff00U+5000U)&&t.state()==S::FaultLocked&&timeout.calls==0);
 Port first,other;E22UartTransition swapped;assert(swapped.begin(first,yes,0));assert(!swapped.step(other,1)&&swapped.state()==S::FaultLocked&&other.calls==0);
 Port lost;E22UartTransition l;assert(l.begin(lost,yes,0));lost.locked=false;assert(!l.step(lost,1)&&l.state()==S::FaultLocked&&lost.calls==0);
}
