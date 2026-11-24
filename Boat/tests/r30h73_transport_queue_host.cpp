// Compile the REAL Boat transport_task_queue.h against deterministic
// thread-safe host substitutes for the FreeRTOS queue and mutex APIs.
// Not an ESP32 FreeRTOS runtime test.
#include <stdint.h>
static void stopInterleave(uint32_t);
#define FLYTEC_HOST_STOP_INTERLEAVE(epoch) stopInterleave(epoch)
#include "transport_task_queue.h"
#undef FLYTEC_HOST_STOP_INTERLEAVE
#include <cassert>
#include <atomic>
#include <thread>
#include <iostream>
#include <set>
#include <mutex>
#include <functional>
#include <chrono>
#include <condition_variable>
static std::mutex seamMutex;
static std::condition_variable seamCv;
static bool seamEnabled=false, seamWaiting=false, seamRelease=false;
static uint32_t seamEpoch=0;
static void stopInterleave(uint32_t epoch) {
 std::unique_lock<std::mutex> lock(seamMutex);
 if (!seamEnabled || epoch!=seamEpoch) return;
 seamWaiting=true; seamCv.notify_all();
 seamCv.wait(lock,[]{return seamRelease;});
}
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
  assert(q.clearStopLatchAfterAuthorization());
 }
 assert(failures.load()==0);
 // R30H-74: two producers, one consumer. No STOP in this phase:
 // contention can reject offers, but may never silently duplicate IDs.
 assert(!q.stopLatched());
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
 // R30H-80: STOP after a consumed STOP but before rearm must defeat rearm.
 assert(q.offerStop({81001}));
 assert(q.receive(c,stop,token) && stop && c.id==81001);
 assert(q.offerStop({81002}));
 assert(!q.clearStopLatchAfterAuthorization());
 assert(q.receive(c,stop,token) && stop && c.id==81002);
 assert(q.clearStopLatchAfterAuthorization());
 // R30H-81: fail-closed clear must preserve a STOP queued concurrently.
 assert(q.offerStop({82001}));
 q.clear();
 assert(q.stopLatched());
 assert(!q.clearStopLatchAfterAuthorization());
 assert(q.receive(c,stop,token) && stop && c.id==82001);
 // clear() advanced the generation, so a stale STOP dequeue alone is
 // insufficient to authorize rearm. This is intentionally fail-closed.
 assert(!q.clearStopLatchAfterAuthorization());
 // R30H-84: adversarial rearm versus fresh STOP. The only legal
 // ordering is rearm-before-STOP (STOP remains latched) or STOP-before-
 // rearm (rearm rejects pending STOP). No execution may be authorized.
 for (int race=0; race<500; ++race) {
  flytec_rtos::CommandHandoff<Cmd,4> concurrent;
  assert(concurrent.begin());
  assert(concurrent.clearStopLatchAfterAuthorization());
  assert(concurrent.offer({uint32_t(90000+race)}));
  Cmd oldCmd{}; bool oldStop=false; uint32_t oldToken=0;
  assert(concurrent.receive(oldCmd,oldStop,oldToken) && !oldStop);
  assert(concurrent.offerStop({uint32_t(91000+race)}));
  Cmd consumed{}; bool consumedStop=false; uint32_t consumedToken=0;
  assert(concurrent.receive(consumed,consumedStop,consumedToken) && consumedStop);
  std::atomic<bool> startRace{false};
  std::atomic<bool> rearmed{false};
  std::thread rearm([&] {
   while (!startRace.load(std::memory_order_acquire)) std::this_thread::yield();
   rearmed.store(concurrent.clearStopLatchAfterAuthorization(),std::memory_order_release);
  });
  std::thread emergency([&] {
   while (!startRace.load(std::memory_order_acquire)) std::this_thread::yield();
   assert(concurrent.offerStop({uint32_t(92000+race)}));
  });
  startRace.store(true,std::memory_order_release);
  rearm.join(); emergency.join();
  assert(concurrent.stopLatched());
  assert(!concurrent.mayExecute(oldToken));
  assert(!concurrent.clearStopLatchAfterAuthorization());
  Cmd pending{}; bool pendingStop=false; uint32_t pendingToken=0;
  assert(concurrent.receive(pending,pendingStop,pendingToken) &&
         pendingStop && pending.id==uint32_t(92000+race));
  (void)rearmed;
 }
 // R30H-93: pause older producer after latch, before writer arbitration.
 {
  flytec_rtos::CommandHandoff<Cmd,4> dual;
  assert(dual.begin() && dual.clearStopLatchAfterAuthorization());
  seamEpoch=dual.currentEpoch()+1; seamEnabled=true;
  bool oldAccepted=true;
  std::thread older([&]{oldAccepted=dual.offerStop({101});});
  {
   std::unique_lock<std::mutex> lock(seamMutex);
   assert(seamCv.wait_for(lock,std::chrono::seconds(5),[]{return seamWaiting;}));
  }
  assert(dual.offerStop({202}));
  {
   std::lock_guard<std::mutex> lock(seamMutex);
   seamRelease=true;
  }
  seamCv.notify_all(); older.join(); seamEnabled=false;
  assert(!oldAccepted && dual.stopLatched());
  assert(!dual.clearStopLatchAfterAuthorization());
  assert(dual.receive(c,stop,token) && stop && c.id==202);
  assert(token==dual.currentEpoch());
  assert(dual.clearStopLatchAfterAuthorization());
 }
 std::cout<<"R30H84 actual header / mock FreeRTOS stress PASS (1000 STOP rounds, 400 delivered IDs, 2 concurrent STOP writers)\n";
}
