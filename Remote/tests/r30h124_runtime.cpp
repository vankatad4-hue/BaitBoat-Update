#include "remote_worker_runtime.h"
#include <cassert>
#include <thread>
using namespace flytec_rtos;
struct Worker {
 bool launch=true,done=true;unsigned starts=0,stops=0;
 RadioWatchdog::State health=RadioWatchdog::State::Starting;
 bool begin(bool authorized){assert(authorized);++starts;done=!launch;return launch;}
 void requestStop(){++stops;}
 bool exited()const{return done;}
 bool permit()const{return health==RadioWatchdog::State::Healthy;}
 RadioWatchdog::State supervise(){return health;}
};
struct Service {
 unsigned calls=0;bool ok=true;
 bool service(uint32_t){assert(remoteI2cAllowed());++calls;return ok;}
};
int main(){
 assert(bindRemoteI2cOwner());
 Worker w;Service s;
 using Runtime=RemoteWorkerRuntime<Worker,Service>;
 Runtime r(w,s);
 assert(!r.begin(false,true)&&!r.begin(true,false));
 std::thread foreign([&]{assert(!r.begin(true,true));assert(!r.tick(0));r.requestStop();});
 foreign.join();
 assert(w.starts==0&&w.stops==0);
 assert(r.begin(true,true)&&!r.begin(true,true));
 assert(r.tick(1)&&!r.permit()&&s.calls==1);
 w.health=RadioWatchdog::State::Healthy;
 assert(r.tick(2)&&r.permit()&&s.calls==2);
 w.health=RadioWatchdog::State::Fault;
 assert(!r.tick(3)&&r.state()==Runtime::State::Retired&&w.stops==1);
 assert(!r.permit()&&!r.exited()&&!r.tick(4)&&s.calls==2);
 w.done=true;assert(r.exited()&&!r.begin(true,true));
 Worker failed;failed.launch=false;Service unused;Runtime bad(failed,unused);
 assert(!bad.begin(true,true)&&bad.state()==Runtime::State::Retired);
 assert(!bad.begin(true,true)&&failed.starts==1);
 Worker serviceFault;Service broken;Runtime f(serviceFault,broken);
 assert(f.begin(true,true));broken.ok=false;
 assert(!f.tick(1)&&f.state()==Runtime::State::Retired&&!f.permit());
 assert(!f.tick(2)&&broken.calls==1);
 Worker stopped;Service idle;Runtime stop(stopped,idle);
 assert(stop.begin(true,true));stop.requestStop();
 assert(stop.state()==Runtime::State::Retired&&!stop.tick(1)&&idle.calls==0);
}
