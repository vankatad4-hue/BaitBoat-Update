#pragma once
#include <atomic>
#include "r30h118_completion_gate.h"
namespace flytec_r30h118 {
// Single application publisher; worker reads only. Zero disables admission.
// A check cannot revoke a driver call already underway.
class WorkerGuard {
 const TicketDomain domain_;
 std::atomic<uint32_t> generation_{0};
public:
 explicit WorkerGuard(TicketDomain domain=TicketDomain::Application):domain_(domain){}
 void disable(){generation_.store(0,std::memory_order_release);}
 void publish(uint32_t generation){generation_.store(generation,std::memory_order_release);}
 bool allows(const Ticket &t,uint32_t queuedAt,uint32_t now,uint32_t maxAge) const {
  const uint32_t active=generation_.load(std::memory_order_acquire);
  return active!=0&&t.domain==domain_&&t.generation==active&&t.epoch!=0&&t.serial!=0&&
         maxAge!=0&&uint32_t(now-queuedAt)<maxAge;
 }
};
}
