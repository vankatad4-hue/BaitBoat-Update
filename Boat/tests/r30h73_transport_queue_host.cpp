// Compile the REAL Boat transport_task_queue.h against deterministic
// thread-safe host substitutes for the FreeRTOS queue and mutex APIs.
// Not an ESP32 FreeRTOS runtime test.
#include "transport_task_queue.h"
#include <cassert>
#include <atomic>
#include <thread>
#include <iostream>
struct Cmd { uint32_t id; };
int main() {
 flytec_rtos::CommandHandoff<Cmd,8> q;
 assert(q.begin());
 assert(q.stopLatched());
 assert(!q.offer({1}));
 assert(q.clearStopLatchAfterAuthorization());
 assert(q.offer({2}));
 Cmd c{}; bool stop=false; uint32_t token=0;
 assert(q.receive(c,stop,token) && !stop && c.id==2);
 assert(q.mayExecute(token));
 assert(q.offerStop({99}));
 assert(q.stopLatched() && !q.mayExecute(token));
 assert(!q.executeIfCurrent(token,[]{ assert(false); }));
 assert(q.receive(c,stop,token) && stop && c.id==99);
 assert(!q.offer({3}));
 assert(q.clearStopLatchAfterAuthorization());
 assert(!q.mayExecute(token));
 // Legacy API cannot return ordinary work.
 assert(q.offer({4}));
 assert(!q.receive(c,stop));
 assert(q.receive(c,stop,token) && c.id==4);
 std::atomic<int> failures{0};
 for(int round=0;round<1000;++round) {
  assert(q.clearStopLatchAfterAuthorization());
  assert(q.offer({uint32_t(round+1000)}));
  assert(q.receive(c,stop,token) && !stop);
  const uint32_t old=token;
  std::thread a([&] { if(!q.offerStop({777})) ++failures; });
  std::thread b([&] { if(q.mayExecute(old) && q.stopLatched()) ++failures; });
  a.join();b.join();
  assert(!q.mayExecute(old));
  assert(q.stopLatched());
  assert(q.receive(c,stop,token) && stop && c.id==777);
 }
 assert(failures.load()==0);
 std::cout<<"R30H73 actual header / mock FreeRTOS host stress PASS (1000 rounds)\n";
}
