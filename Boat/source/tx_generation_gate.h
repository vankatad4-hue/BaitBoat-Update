#pragma once
#include <atomic>
#include <stdint.h>
// Dormant admission gate. One application owner changes generation.
// One PHY owner uses tryAdmit. Transition is nonblocking and must be retried
// until accepted. No driver callback may block or reenter this gate.
namespace flytec_rtos {
class TxGenerationGate {
public:
 using Admit=bool(*)(void*);
 enum class Result { Busy, Stale, Rejected, Accepted };
 bool transition(uint32_t next){
  if(lock_.test_and_set(std::memory_order_acquire))return false;
  const bool valid=next>generation_;
  if(valid)generation_=next;
  lock_.clear(std::memory_order_release);return valid;
 }
 Result tryAdmit(uint32_t expected,Admit admit,void* ctx){
  if(lock_.test_and_set(std::memory_order_acquire))return Result::Busy;
  Result r=Result::Stale;
  if(expected==generation_&&generation_!=0)
   r=admit&&admit(ctx)?Result::Accepted:Result::Rejected;
  lock_.clear(std::memory_order_release);return r;
 }
private:
 std::atomic_flag lock_=ATOMIC_FLAG_INIT;
 uint32_t generation_=0;
};
}
