#pragma once
#include "r30h118_spsc_queue.h"
#include "r30h118_completion_gate.h"
namespace flytec_r30h118 {
enum class PhyOutcome:uint8_t { LocalComplete, Failed, Uncertain };
template<class Frame> class PhyEvents {
public:
 struct Rx { Frame frame; uint32_t capturedAt; };
 struct Health { uint32_t observedAt; bool configured,ready,fault,pending; };
 struct Outcome { Ticket ticket; uint32_t observedAt; PhyOutcome result; };
private:
 SpscQueue<Rx,5> rx_;
 SpscQueue<Health,5> health_;
 SpscQueue<Outcome,5> outcomes_;
public:
 // Worker publishes; application consumes. Separate instance per physical link.
 // False means full: caller retains critical events and retries; no overwrite.
 bool publishRx(const Rx &v){return rx_.push(v);}
 bool publishHealth(const Health &v){return health_.push(v);}
 bool publishOutcome(const Outcome &v){return outcomes_.push(v);}
 bool takeRx(Rx &v){return rx_.pop(v);}
 bool takeHealth(Health &v){return health_.pop(v);}
 bool takeOutcome(Outcome &v){return outcomes_.pop(v);}
};
inline bool sameTicket(const Ticket &a,const Ticket &b){
 return a.generation==b.generation&&a.serial==b.serial&&a.epoch==b.epoch;
}
}
