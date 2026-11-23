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
#include <condition_variable>
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
 assert(!q.clearStopLatchAfterAuthorization()); // STOP still pending
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
 // R30H-75: dequeue before STOP, delay preparation until STOP latches.
 assert(q.offer({60000}));
 assert(q.receive(c,stop,token) && !stop && c.id==60000);
 const uint32_t heldToken=token;
 std::mutex barrierMutex;
 std::condition_variable barrierCv;
 bool waiting=false,release=false;
 std::atomic<int> executed{0};
 std::thread held([&] {
  {
   std::unique_lock<std::mutex> lock(barrierMutex);
   waiting=true;barrierCv.notify_one();
   barrierCv.wait(lock,[&]{return release;});
  }
  (void)q.executeIfCurrent(heldToken,[&]{++executed;});
 });
 {
  std::unique_lock<std::mutex> lock(barrierMutex);
  barrierCv.wait(lock,[&]{return waiting;});
 }
 assert(q.offerStop({59999}));
 {
  std::lock_guard<std::mutex> lock(barrierMutex);
  release=true;
 }
 barrierCv.notify_one();
 held.join();
 assert(executed.load()==0 && !q.mayExecute(heldToken));
 assert(q.receive(c,stop,token) && stop && c.id==59999);
 // STOP under simultaneous ordinary producer pressure must invalidate all
 // old tokens. A fresh STOP is eventually retrievable.
 assert(q.offerStop({50000}));
 assert(!q.offer({50001}));
 assert(q.receive(c,stop,token) && stop && c.id==50000);
 // R30H-77: concurrent STOP writers must leave STOP latched, regardless
 // of which caller wins the single overwrite slot. Losing caller retries
 // only after the first writer has completed.
 assert(q.clearStopLatchAfterAuthorization());
 std::atomic<bool> fire{false};
 std::atomic<int> stopAccepted{0};
 auto stopProducer=[&](uint32_t id) {
  while(!fire.load(std::memory_order_acquire)) std::this_thread::yield();
  for(int attempt=0;attempt<100000;++attempt) {
   if(q.offerStop({id})) { ++stopAccepted; return; }
   std::this_thread::yield();
  }
  assert(false && "STOP producer retry budget exhausted");
 };
 std::thread stopA(stopProducer,71001U);
 std::thread stopB(stopProducer,71002U);
 fire.store(true,std::memory_order_release);
 stopA.join();stopB.join();
 assert(stopAccepted.load()==2 && q.stopLatched());
 assert(!q.clearStopLatchAfterAuthorization()); // pending STOP
 assert(q.receive(c,stop,token) && stop);
 assert(c.id==71001U || c.id==71002U);
 assert(q.stopLatched());
 assert(q.clearStopLatchAfterAuthorization());
 std::cout<<"R30H77 actual header / mock FreeRTOS stress PASS (1000 STOP rounds, 400 delivered IDs, 2 concurrent STOP writers)\n";
}
