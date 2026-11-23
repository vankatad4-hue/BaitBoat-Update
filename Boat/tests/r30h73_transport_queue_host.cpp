// Compile the REAL Boat transport_task_queue.h against deterministic
// thread-safe host substitutes for the FreeRTOS queue and mutex APIs.
// Not an ESP32 FreeRTOS runtime test.
#include "transport_task_queue.h"
#include <cassert>
#include <atomic>
#include <thread>
#include <iostream>
#include <set>
#include <mutex>
#include <functional>
#include <chrono>
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
  std::thread b([&] { (void)q.mayExecute(old); });
  a.join();b.join();
  assert(!q.mayExecute(old));
  assert(q.stopLatched());
  assert(q.receive(c,stop,token) && stop && c.id==777);
 }
 assert(failures.load()==0);
 // R30H-74: two producers, one consumer. No STOP in this phase:
 // contention can reject offers, but may never silently duplicate IDs.
 assert(q.clearStopLatchAfterAuthorization());
 std::atomic<bool> doneA{false},doneB{false};
 std::atomic<int> admitted{0};
 std::set<uint32_t> received;
 std::mutex receivedMutex;
 auto produce=[&](uint32_t base,std::atomic<bool>& done) {
  for(uint32_t i=0;i<200;++i) {
   const Cmd item{base+i};
   while(!q.offer(item)) std::this_thread::yield();
   ++admitted;
  }
  done.store(true);
 };
 std::thread producerA(produce,10000U,std::ref(doneA));
 std::thread producerB(produce,20000U,std::ref(doneB));
 std::thread consumer([&] {
  const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
  while(!doneA.load() || !doneB.load() || received.size()<400U) {
   assert(std::chrono::steady_clock::now()<deadline && "consumer stalled");
   Cmd item{}; bool stopFlag=false; uint32_t e=0;
   if(q.receive(item,stopFlag,e)) {
    assert(!stopFlag && q.mayExecute(e));
    std::lock_guard<std::mutex> lock(receivedMutex);
    assert(received.insert(item.id).second);
   } else std::this_thread::yield();
  }
 });
 producerA.join();producerB.join();consumer.join();
 assert(admitted.load()==400 && received.size()==400);
 // STOP under simultaneous ordinary producer pressure must invalidate all
 // old tokens. A fresh STOP is eventually retrievable.
 assert(q.offerStop({50000}));
 assert(!q.offer({50001}));
 assert(q.receive(c,stop,token) && stop && c.id==50000);
 std::cout<<"R30H74 actual header / mock FreeRTOS stress PASS (1000 STOP rounds, 400 delivered IDs)\n";
}
