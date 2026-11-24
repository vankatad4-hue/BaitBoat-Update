#pragma once
#include "r30h118_transport.h"
namespace flytec_r30h120 {
// Worker-local state. Owner never invokes step or physical callbacks.
template<class Frame> class PhyService {
 using Bus=flytec_r30h118::Transport<Frame>;
 Bus &bus_;
 typename Bus::Request request_{};
 bool held_=false,admitted_=false;
public:
 explicit PhyService(Bus &bus):bus_(bus){}
 template<class Admit> void step(uint32_t now,uint32_t maxAge,bool enabled,Admit admit){
  if(held_){if(!bus_.complete(request_,admitted_))return;held_=false;}
  if(!enabled||!bus_.take(request_))return;
  admitted_=bus_.allows(request_,now,maxAge)&&admit(request_);
  held_=!bus_.complete(request_,admitted_);
 }
 bool resultPending()const{return held_;}
};
}
