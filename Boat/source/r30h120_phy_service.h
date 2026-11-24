#pragma once
#include "r30h118_transport.h"
#include "r30h120_handshake_lane.h"
namespace flytec_r30h120 {
// Worker-local state. Owner never invokes step or physical callbacks.
template<class Frame> class PhyService {
 using Bus=flytec_r30h118::Transport<Frame>;
 Bus &bus_;
 typename Bus::Request request_{};
 bool held_=false,admitted_=false;
public:
 explicit PhyService(Bus &bus):bus_(bus){}
 bool flush(){if(held_&&bus_.complete(request_,admitted_))held_=false;return !held_;}
 template<class Admit> void step(uint32_t now,uint32_t maxAge,bool enabled,Admit admit){
  if(!flush())return;
  if(!enabled||!bus_.take(request_))return;
  admitted_=bus_.allows(request_,now,maxAge)&&admit(request_);
  held_=!bus_.complete(request_,admitted_);
 }
 bool resultPending()const{return held_;}
};

// One worker-local arbiter per physical link. Both admission callbacks must
// target the same exclusive adapter. Terminal outcome routing is separate.
template<class Frame> class SharedPhyService {
 PhyService<Frame> app_;
 HandshakeService<Frame> handshake_;
 const SharedPriority &priority_;
public:
 SharedPhyService(flytec_r30h118::Transport<Frame>& app,HandshakeLane<Frame>& hs,const SharedPriority &priority):app_(app),handshake_(hs),priority_(priority){}
 template<class Policy,class AppAdmit,class HsAdmit>
 void step(uint32_t now,uint32_t age,bool busy,bool enabled,Policy policy,AppAdmit appAdmit,HsAdmit hsAdmit){
  const bool appClear=app_.flush();
  const bool hsClear=handshake_.flush();
  if(!appClear||!hsClear)return;
  priority_.service(busy,enabled,[&](TxPriority selected){
   app_.step(now,age,true,[&](const typename flytec_r30h118::Transport<Frame>::Request &r){
    // A stale advisory STOP mask must never admit an ordinary request.
    return (selected==TxPriority::Stop)==r.stop&&appAdmit(r);
   });
   return true;
  },[&](){return handshake_.step(now,age,true,policy,hsAdmit);});
 }
};
}
