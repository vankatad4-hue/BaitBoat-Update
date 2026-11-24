#include "remote_i2c_dispatcher.h"
#include "radio_watchdog.h"
#include <cassert>
using namespace flytec_rtos;
static uint32_t us=0;
static uint32_t clockUs(){return us;}
using D=RemoteI2cDispatcher;
static bool execute(void*,D::Operation op){us+=op==D::Operation::PollPrimary?7:13;return true;}
struct Fake{void tick(uint32_t){us+=5;}};
int main(){
 LatencyStats a;a.observe(2);a.observe(7);a.observe(3);assert(a.count==3&&a.max==7&&a.mean()==4);
 PollTiming p;p.begin(UINT32_MAX-3);p.end(2);p.begin(10);p.end(14);
 assert(p.duration.count==2&&p.duration.max==6&&p.gap.max==14);
 Fake f;TimedPoller<Fake> t(f,clockUs);us=100;t.tick(1);us=120;t.tick(2);
 assert(t.timing().duration.mean()==5&&t.timing().gap.max==20);
 assert(bindRemoteI2cOwner());D d(clockUs);assert(d.begin());
 using O=D::Operation;assert(d.submit({1,100,O::PollPrimary}));assert(d.submit({2,101,O::PollBackup}));
 assert(d.service(110,execute,nullptr));assert(d.service(111,execute,nullptr));
 assert(d.submit({3,112,O::PollPrimary}));assert(d.service(212,execute,nullptr));
 const auto pm=d.metrics(O::PollPrimary),bm=d.metrics(O::PollBackup);
 assert(pm.queueAgeMs.count==2&&pm.queueAgeMs.max==100&&pm.executionUs.duration.count==1&&pm.executionUs.duration.max==7);
 assert(bm.queueAgeMs.max==10&&bm.executionUs.duration.max==13);
 RadioWatchdog watchdog(500);watchdog.reset(0,0);
 assert(watchdog.update(1,1,true,false)==RadioWatchdog::State::Healthy);
 assert(watchdog.update(500,1,true,false)==RadioWatchdog::State::Healthy);
 assert(watchdog.update(501,1,true,false)==RadioWatchdog::State::Fault);
}
