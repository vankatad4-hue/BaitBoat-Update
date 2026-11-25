#pragma once
#include <atomic>
#include <stdint.h>
// Dormant. One application owner for transition/latchStop/rearm.
// One PHY owner. Callback must be nonblocking and non-reentrant.
// An already admitted physical TX cannot be revoked by this gate.
namespace flytec_rtos {
class TxGenerationGate {
public:
 using Admit=bool(*)(void*);
 enum class Result { Busy, Stale, Inhibited, Rejected, Accepted };
 bool transition(uint32_t next){
  if(lock_.test_and_set(std::memory_order_acquire))return false;
  const bool valid=!pending_.load()&&next>generation_;
  if(valid)generation_=next;
  unlock();return valid;
 }
 // Publish inhibition before attempting the lock; retry same next on contention.
 bool latchStop(uint32_t next){
  pending_.store(true);stopped_.store(true);
  if(lock_.test_and_set(std::memory_order_acquire))return false;
  const bool valid=next>generation_;
  if(valid){generation_=next;pending_.store(false);}
  unlock();return valid;
 }
 bool rearm(uint32_t next,bool authorized){
  if(!authorized||lock_.test_and_set(std::memory_order_acquire))return false;
  const bool valid=stopped_.load()&&!pending_.load()&&next>generation_;
  if(valid){generation_=next;stopped_.store(false);}
  unlock();return valid;
 }
 bool stopped()const{return stopped_.load();}
 Result tryAdmit(uint32_t expected,Admit admit,void* ctx,bool critical=false){
  if(lock_.test_and_set(std::memory_order_acquire))return Result::Busy;
  Result r=Result::Stale;
  if(pending_.load())r=Result::Busy;
  else if(expected==generation_&&generation_!=0){
   if(stopped_.load()&&!critical)r=Result::Inhibited;
   else r=admit&&admit(ctx)?Result::Accepted:Result::Rejected;
  }
  unlock();return r;
 }
private:
 void unlock(){lock_.clear(std::memory_order_release);}
 std::atomic_flag lock_=ATOMIC_FLAG_INIT;
 std::atomic<bool> pending_{false},stopped_{false};
 uint32_t generation_=0;
};
}
