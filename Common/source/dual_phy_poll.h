#pragma once
#include <atomic>
#include <stdint.h>
#include "radio_frame_handoff.h"
#include "tx_generation_gate.h"
// Dormant single-owner polling harness. No driver initialization, GPIO,
// session authentication, actuator commands or UI here. Both adapters
// must have bounded calls. A blocked shared bus still defeats isolation.
namespace flytec_rtos {
template<class Primary,class Backup> class DualPhyPoll {
public:
 DualPhyPoll(Primary& p,Backup& b,RadioFrameHandoff& h,TxGenerationGate& g):
  primary_(p),backup_(b),handoff_(h),gate_(g){}
 DualPhyPoll(const DualPhyPoll&)=delete;
 DualPhyPoll& operator=(const DualPhyPoll&)=delete;
 // One invocation per worker iteration; no short-circuit on link failure.
 void tick(uint32_t now){
  bool p,b;
  if(reverse_){
   b=backup_.tick(handoff_,gate_,now);
   p=primary_.tick(handoff_,gate_,now);
  }else{
   p=primary_.tick(handoff_,gate_,now);
   b=backup_.tick(handoff_,gate_,now);
  }
  reverse_=!reverse_;
  // Snapshot of last completed poll only, not RF connectivity or ACK.
  status_.store(4U|(p?1U:0U)|(b?2U:0U),std::memory_order_release);
 }
 struct Status{bool observed;bool primaryPollOk;bool backupPollOk;};
 Status status()const{
  const auto v=status_.load(std::memory_order_acquire);
  return {(v&4U)!=0,(v&1U)!=0,(v&2U)!=0};
 }
private:
 Primary& primary_;Backup& backup_;
 RadioFrameHandoff& handoff_;TxGenerationGate& gate_;
 bool reverse_=false;
 std::atomic<uint32_t> status_{0};
};
}
