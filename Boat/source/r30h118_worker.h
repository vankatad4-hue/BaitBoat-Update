#pragma once
#include "r30h118_transport.h"
namespace flytec_r30h118 {
// One-shot lifecycle: construct before task start, destroy only after join.
// Owner requests shutdown; worker alone services driver and pending result.
template<class Frame> class Worker {
 using Bus=Transport<Frame>;
 Bus &bus_;
 typename Bus::Request pending_{};
 bool hasResult_=false, admitted_=false;
 std::atomic<bool> stopping_{false}, exited_{false};
public:
 explicit Worker(Bus &bus):bus_(bus){}
 // Owner must stop submissions before calling this.
 void requestStop(){bus_.invalidate(0);stopping_.store(true,std::memory_order_release);}
 bool exited()const{return exited_.load(std::memory_order_acquire);}
 template<class Admit> void step(uint32_t now,uint32_t maxAge,Admit admit){
  if(exited())return;
  if(hasResult_){
   if(!bus_.complete(pending_,admitted_))return;
   hasResult_=false;
  }
  if(stopping_.load(std::memory_order_acquire)){
   exited_.store(true,std::memory_order_release);return;
  }
  if(!bus_.take(pending_))return;
  admitted_=false;
  if(!stopping_.load(std::memory_order_acquire)&&bus_.allows(pending_,now,maxAge))
   admitted_=admit(pending_.frame);
  hasResult_=true;
  if(bus_.complete(pending_,admitted_))hasResult_=false;
 }
};
}
