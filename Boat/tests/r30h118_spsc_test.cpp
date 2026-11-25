#include "boat_radio_transport.h"
#include <cassert>
#include <thread>
#include <cstdint>
using namespace flytec_r30h118;
int main(){
 SpscQueue<uint32_t,4> q;
 uint32_t v=0;
 assert(!q.pop(v));
 for(uint32_t i=0;i<3;++i)assert(q.push(i));
 assert(!q.push(3));
 for(uint32_t i=0;i<3;++i){assert(q.pop(v));assert(v==i);}
 std::thread producer([&]{for(uint32_t i=0;i<100000;++i)while(!q.push(i))std::this_thread::yield();});
 for(uint32_t i=0;i<100000;++i){while(!q.pop(v))std::this_thread::yield();assert(v==i);}
 producer.join();
 using Lane=RadioHandoff<uint32_t>;
 Lane lane; SpscQueue<Lane::Request,4> requests;
 SpscQueue<Lane::Completion,4> results;
 Lane::Request a{},b{},r{};
 assert(lane.invalidate(1));assert(lane.prepare(1,a));assert(requests.push(a));
 assert(lane.invalidate(1));assert(lane.prepare(2,b));assert(requests.push(b));
 std::thread worker([&]{for(int i=0;i<2;++i){assert(requests.pop(r));assert(results.push({r.ticket,true}));}});
 worker.join();
 Lane::Completion c{};bool admitted=false;
 assert(results.pop(c));assert(!lane.accept(c,admitted));assert(!admitted);
 assert(results.pop(c));assert(lane.accept(c,admitted));assert(admitted);
}
