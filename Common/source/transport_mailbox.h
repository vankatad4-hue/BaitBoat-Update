#pragma once
// R30H-61: fixed-capacity, allocation-free transport-to-application mailbox.
// This is the single-owner queue primitive; synchronize calls before using
// across FreeRTOS tasks. STOP is out-of-band and never enters this FIFO.
#include <stddef.h>
#include <stdint.h>
namespace flytec_mailbox {
template <typename T, size_t Capacity> class BoundedMailbox {
 static_assert(Capacity > 0, "Mailbox capacity must be positive");
public:
 bool push(const T& value) {
  if (count_ == Capacity) { ++dropped_; return false; }
  entries_[tail_] = value;
  tail_ = (tail_ + 1U) % Capacity;
  ++count_;
  return true;
 }
 bool pop(T& out) {
  if (!count_) return false;
  out = entries_[head_];
  head_ = (head_ + 1U) % Capacity;
  --count_;
  return true;
 }
 void clear() { head_ = tail_ = count_ = 0; }
 size_t size() const { return count_; }
 static constexpr size_t capacity() { return Capacity; }
 uint32_t dropped() const { return dropped_; }
private:
 T entries_[Capacity]{};
 size_t head_ = 0, tail_ = 0, count_ = 0;
 uint32_t dropped_ = 0;
};
} // namespace flytec_mailbox
