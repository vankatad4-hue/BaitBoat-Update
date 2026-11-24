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
 // Separate fixture for deterministic timeout with a late result.
 D e;assert(e.begin());S z(e);Counts n;
 z.tick(1000);assert(e.service(1001,run,&n));z.tick(1002);
 assert(z.status(P)==S::State::Completed&&z.status(B)==S::State::Waiting);
 z.tick(1100);assert(e.service(1101,run,&n)); // expired backup queued result
 assert(e.service(1102,run,&n));z.tick(1103); // consume primary result, fault expired backup
 assert(z.status(B)==S::State::Fault);
 z.tick(1104);
 assert(e.service(1105,run,&n));z.tick(1106);
 assert(z.status(P)==S::State::Completed&&n.p==3&&n.b==0);
 // Exact response timeout, then an expired result arrives too late.
 D f;assert(f.begin());S y(f);Counts m;
 y.tick(2000);assert(f.service(2001,run,&m));y.tick(2002);
 y.tick(2200);assert(y.status(B)==S::State::Fault);
 assert(f.service(2201,run,&m));y.tick(2202);
 assert(y.status(B)==S::State::Fault&&m.b==0);
}
