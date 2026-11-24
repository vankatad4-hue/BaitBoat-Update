#pragma once
#include <atomic>
#include <stddef.h>
#include <type_traits>
namespace flytec_r30h118 {
// Exactly one producer and one consumer. Never reset while either is active.
// Stale entries are drained and rejected by owner-side ticket validation.
template<class T, size_t N> class SpscQueue {
 static_assert(N>=2,"one slot reserved");
 static_assert(std::is_trivially_copyable<T>::value,"copied messages only");
 T slots_[N]{};
 std::atomic<size_t> head_{0},tail_{0};
public:
 bool push(const T &v){
  const size_t h=head_.load(std::memory_order_relaxed), next=(h+1)%N;
  if(next==tail_.load(std::memory_order_acquire))return false;
  slots_[h]=v;head_.store(next,std::memory_order_release);return true;
 }
 bool pop(T &v){
  const size_t t=tail_.load(std::memory_order_relaxed);
  if(t==head_.load(std::memory_order_acquire))return false;
  v=slots_[t];tail_.store((t+1)%N,std::memory_order_release);return true;
 }
};
}
