#include <atomic>
#include <cassert>
#include <cstdint>
#include <thread>
#include <mutex>
#include <iostream>
// R30H-72 portable policy model, NOT a FreeRTOS queue or physical-output test.
struct Gate {
 std::atomic<bool> stop{true};
 std::atomic<uint32_t> epoch{0};
 std::mutex mutex;
 void rearm() { std::lock_guard<std::mutex> l(mutex); epoch.fetch_add(1); stop.store(false); }
 uint32_t token() const { return epoch.load(); }
 void stopNow() { stop.store(true); epoch.fetch_add(1); }
 bool permitted(uint32_t t) const { return !stop.load() && epoch.load()==t; }
 template<class F> bool prepare(uint32_t t,F f) {
  std::lock_guard<std::mutex> l(mutex);
  if(!permitted(t)) return false;
  f(); return true;
 }
};
int main() {
 Gate g;
 g.rearm(); auto stale=g.token();
 g.stopNow(); assert(!g.permitted(stale));
 g.rearm(); assert(!g.permitted(stale));
 std::atomic<int> accepted{0};
 for(int round=0;round<1000;++round) {
  g.rearm(); const auto t=g.token();
  std::thread producer([&]{g.stopNow();});
  std::thread consumer([&]{g.prepare(t,[&]{accepted.fetch_add(1);});});
  producer.join(); consumer.join();
  assert(!g.permitted(t));
 }
 std::cout<<"R30H72 portable epoch/STOP interleavings PASS (1000 rounds), prepared="<<accepted.load()<<"\n";
}
