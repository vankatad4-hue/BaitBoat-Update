#include "lora_link_policy.h"
#include <cassert>
#include <cstring>
#include <vector>
using namespace flytec_rtos;
struct Mock {
 bool locked=true,config=false,fault=false,stoppedClock=false,extra=false,shortReply=false,wrongToken=false,dropIsolation=false,busy=false;
 uint32_t now=0,baud=9600,releaseAt=0;uint8_t reg=0x67;unsigned writes=0,arms=0,index=0;
 std::vector<uint8_t> reply;
 bool commissioningAllowed()const{return locked&&!fault;}
 void commissioningLatchFault(){fault=true;}
 uint32_t commissioningNow()const{return now;}
 void commissioningWait(){if(!stoppedClock)++now;if(dropIsolation&&now>=8)locked=false;}
 bool commissioningHostBaud(uint32_t,uint32_t b,uint32_t){baud=b;return true;}
 bool commissioningModeMatches(bool c){return commissioningAllowed()&&config==c;}
 bool commissioningModeVerified(bool c,uint32_t){config=c;return true;}
 bool commissioningRxCount(unsigned& n){n=uint32_t(now-releaseAt)<0x80000000U?unsigned(reply.size()-index):0;return true;}
 bool commissioningReadByte(uint8_t& b){if(index>=reply.size())return false;b=reply[index++];return true;}
 bool commissioningTxIdle(){return !busy;}
 void feed(const uint8_t* b,unsigned n){reply.assign(b,b+n);index=0;releaseAt=now+3;if(shortReply)reply.pop_back();if(extra)reply.push_back(0xaa);}
 bool commissioningConfigWrite(const uint8_t* cmd,unsigned n){assert(config&&baud==9600);++writes;
  if(n==3){const uint8_t v[]={0xc1,0,9,0,0,0,reg,0xc3,0x12,0,0,0};feed(v,12);}
  else{reg=cmd[3];const uint8_t v[]={0xc1,3,1,reg};feed(v,4);}return true;
 }
 static bool arm(void* p,uint32_t b,const uint8_t* token,unsigned n,uint32_t){auto& m=*static_cast<Mock*>(p);assert(!m.config&&m.baud==b&&n==8);++m.arms;m.feed(token,n);if(m.wrongToken)m.reply[0]^=1;return true;}
};
int main(){
 using P=E22OfflinePort<Mock>;using S=E22UartTransition::State;
 const uint8_t token[]={1,3,5,7,9,2,4,6};const E22CommissioningPermit permit{true,true,true,true,true};
 auto run=[&](Mock& m,P& p,E22UartTransition& t){assert(t.begin(p,permit,m.now));while(t.step(p,m.now))++m.now;};
 Mock m;P p(m,0,&Mock::arm,&m,token);E22UartTransition t;run(m,p,t);
 assert(t.state()==S::VerifiedLocked&&t.verifiedBaud()==115200&&m.writes==3&&m.arms==1&&m.locked&&!m.fault);
 assert(!p.receiveFixtureVerified(115200,500)&&m.fault&&m.arms==1);
 for(unsigned fault=0;fault<5;++fault){Mock q;if(fault==0)q.extra=true;else if(fault==1)q.shortReply=true;else if(fault==2)q.stoppedClock=true;else if(fault==3)q.wrongToken=true;else q.dropIsolation=true;
  P port(q,0,&Mock::arm,&q,token);E22UartTransition tr;run(q,port,tr);assert(tr.state()==S::FaultLocked&&q.fault&&!tr.verifiedBaud());}
 Mock stale;P sp(stale,0,&Mock::arm,&stale,token);assert(sp.modeVerified(true,500)&&sp.baudVerified(9600,500));stale.reply={0xc1};
 const uint8_t read[]={0xc1,0,9};uint8_t out[12]{};assert(!sp.exchangeExact(read,3,out,12,500)&&stale.writes==0);
 Mock invalid;P ip(invalid,0,&Mock::arm,&invalid,token);assert(ip.modeVerified(true,500)&&ip.baudVerified(9600,500));
 const uint8_t persistent[]={0xc0,3,1,0xe7};assert(!ip.exchangeExact(persistent,4,out,4,500)&&invalid.writes==0);
 Mock busy;P bp(busy,0,&Mock::arm,&busy,token);assert(bp.modeVerified(true,500)&&bp.baudVerified(9600,500));busy.busy=true;assert(!bp.exchangeExact(read,3,out,12,500)&&busy.writes==0);
 Mock wrong;P wp(wrong,0,&Mock::arm,&wrong,token);assert(wp.modeVerified(false,500));assert(!wp.exchangeExact(read,3,out,12,500)&&wrong.writes==0);
 // Fresh offline recovery: accept only a recognized full profile, never
 // blind writes. Restore temporary candidate to baseline, verify fresh RX.
 Mock recovery;recovery.reg=0xe7;recovery.baud=115200;
 P rp(recovery,0,&Mock::arm,&recovery,token);E22UartTransition rt;
 assert(rt.begin(rp,permit,recovery.now,true));while(rt.step(rp,recovery.now))++recovery.now;
 assert(rt.state()==S::VerifiedLocked&&rt.verifiedBaud()==9600&&recovery.reg==0x67&&recovery.writes==3&&recovery.arms==1);
 Mock already;P ap(already,0,&Mock::arm,&already,token);E22UartTransition at;
 assert(at.begin(ap,permit,already.now,true));while(at.step(ap,already.now))++already.now;
 assert(at.verifiedBaud()==9600&&already.writes==2); // two reads, no write
 Mock unknown;unknown.reg=0x27;P up(unknown,0,&Mock::arm,&unknown,token);E22UartTransition ut;
 assert(ut.begin(up,permit,unknown.now,true));while(ut.step(up,unknown.now))++unknown.now;
 assert(ut.state()==S::FaultLocked&&unknown.writes==1&&unknown.reg==0x27&&unknown.fault);
 Mock blind;P bl(blind,0,&Mock::arm,&blind,token);assert(bl.modeVerified(true,500)&&bl.baudVerified(9600,500));
 const uint8_t temporary[]={0xc2,3,1,0x67};assert(!bl.exchangeExact(temporary,4,out,4,500)&&blind.writes==0);
 Mock repeat;P re(repeat,0,&Mock::arm,&repeat,token);assert(re.modeVerified(true,500)&&re.baudVerified(9600,500));
 assert(re.exchangeExact(read,3,out,12,500)&&re.exchangeExact(temporary,4,out,4,500));
 assert(!re.exchangeExact(temporary,4,out,4,500)&&repeat.writes==2);
 Mock wrap;wrap.now=0xfffffff0;P wrapPort(wrap,0,&Mock::arm,&wrap,token);E22UartTransition wrapTransition;run(wrap,wrapPort,wrapTransition);assert(wrapTransition.verifiedBaud()==115200);
}
