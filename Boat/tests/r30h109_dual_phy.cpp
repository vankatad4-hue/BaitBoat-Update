#include "dual_phy_worker.h"
#include <cassert>
#include <thread>
#include <chrono>
using namespace flytec_rtos;
struct Fake {
 unsigned calls=0;bool ok=true;
 bool tick(RadioFrameHandoff&,TxGenerationGate&,uint32_t){++calls;return ok;}
};
static uint32_t clockMs(){return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
int main(){
 RadioFrameHandoff h;assert(h.begin());TxGenerationGate g;assert(g.transition(1));
 Fake p,b;DualPhyPoll<Fake,Fake> poll(p,b,h,g);
 assert(!poll.status().observed);
 p.ok=false;poll.tick(1);assert(p.calls==1&&b.calls==1);
 auto s=poll.status();assert(s.observed&&!s.primaryPollOk&&s.backupPollOk);
 p.ok=true;b.ok=false;poll.tick(2);assert(p.calls==2&&b.calls==2);
 s=poll.status();assert(s.primaryPollOk&&!s.backupPollOk);
 DualPhyWorker<decltype(poll)> worker(poll,clockMs);
 assert(!worker.begin(false)&&!worker.permit());
 assert(worker.begin(true));assert(!worker.begin(true));
 auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(worker.supervise()!=RadioWatchdog::State::Healthy){
  assert(std::chrono::steady_clock::now()<deadline);vTaskDelay(1);
 }
 assert(worker.permit());worker.stop();assert(!worker.permit());
 assert(worker.supervise()==RadioWatchdog::State::Fault);assert(!worker.begin(true));
 assert(p.calls==b.calls&&p.calls>2);
 while(activeTasks.load())vTaskDelay(1);
}
