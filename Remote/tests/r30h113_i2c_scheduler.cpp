#include "remote_i2c_request_scheduler.h"
#include <cassert>
using D=flytec_rtos::RemoteI2cDispatcher;
using S=flytec_rtos::RemoteI2cRequestScheduler;
constexpr auto P=D::Operation::PollPrimary;
constexpr auto B=D::Operation::PollBackup;
struct Counts {unsigned p=0,b=0;};
static bool run(void* ptr,D::Operation op){
 auto& c=*static_cast<Counts*>(ptr);
 if(op==P)++c.p;else ++c.b;
 return true;
}
int main(){
 assert(flytec_rtos::bindRemoteI2cOwner());
 D d;assert(d.begin());S s(d);Counts c;
 s.tick(0);
 for(unsigned i=1;i<20;++i)s.tick(i);
 assert(s.status(P)==S::State::Waiting&&s.status(B)==S::State::Waiting);
 assert(d.service(20,run,&c));assert(d.service(21,run,&c));
 assert(d.service(22,run,&c)&&c.p==1&&c.b==1);
 s.tick(23);assert(s.status(P)==S::State::Completed&&s.status(B)==S::State::Completed);
 s.tick(24);assert(d.service(25,run,&c));s.tick(26);
 assert(s.status(P)==S::State::Completed&&s.status(B)==S::State::Waiting);
 // Let backup time out while continuing to service primary.
 for(uint32_t t=27;t<224;++t){s.tick(t);assert(d.service(t,run,&c));}
 // Separate fixture for deterministic timeout with a late result.
 D e;assert(e.begin());S z(e);Counts n;
 z.tick(1000);assert(e.service(1001,run,&n));z.tick(1002);
 assert(z.status(P)==S::State::Completed&&z.status(B)==S::State::Waiting);
 z.tick(1200);assert(z.status(B)==S::State::Fault);
 assert(e.service(1201,run,&n)); // backup expired; result cannot revive it
 z.tick(1202);assert(z.status(B)==S::State::Fault);
 assert(e.service(1203,run,&n));z.tick(1204);
 assert(z.status(P)==S::State::Completed&&n.p==2&&n.b==0);
}
