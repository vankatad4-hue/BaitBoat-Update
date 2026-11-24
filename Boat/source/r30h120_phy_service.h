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
 typename Bus::Request stopRequest_{};
 bool stopHeld_=false,stopAdmitted_=false;
public:
 explicit PhyService(Bus &bus):bus_(bus){}
 bool flush(){if(held_&&bus_.complete(request_,admitted_))held_=false;return !held_;}
 bool flushStop(){if(stopHeld_&&bus_.complete(stopRequest_,stopAdmitted_))stopHeld_=false;return !stopHeld_;}
 template<class Admit> void step(uint32_t now,uint32_t maxAge,bool enabled,Admit admit){
  if(!flush())return;
  if(!enabled||!bus_.take(request_))return;
  admitted_=bus_.allows(request_,now,maxAge)&&admit(request_);
  held_=!bus_.complete(request_,admitted_);
 }
 template<class Admit> void stepStop(uint32_t now,uint32_t age,Admit admit){
  if(!flushStop()||!bus_.takeStop(stopRequest_))return;
  stopAdmitted_=bus_.allows(stopRequest_,now,age)&&admit(stopRequest_);
  stopHeld_=!bus_.complete(stopRequest_,stopAdmitted_);
 }
 bool resultPending()const{return held_||stopHeld_;}
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
  const bool stopClear=app_.flushStop();
  const bool appClear=app_.flush();
  const bool hsClear=handshake_.flush();
  // A held handshake admission result must not block the independent STOP lane.
  if(!stopClear)return;
  if(!appClear&&choosePriority(priority_.snapshot())!=TxPriority::Stop)return;
  if(!hsClear&&choosePriority(priority_.snapshot())!=TxPriority::Stop)return;
  priority_.service(busy,enabled,[&](TxPriority selected){
   if(selected==TxPriority::Stop){app_.stepStop(now,age,[&](const typename flytec_r30h118::Transport<Frame>::Request &r){return appAdmit(r);});return true;}
   if(!appClear||!hsClear)return false;
   app_.step(now,age,true,[&](const typename flytec_r30h118::Transport<Frame>::Request &r){
    // A stale advisory STOP mask must never admit an ordinary request.
    return (selected==TxPriority::Stop)==r.stop&&
     choosePriority(priority_.snapshot())==selected&&appAdmit(r);
   });
   return true;
  },[&](){if(!appClear||!hsClear)return false;
  return handshake_.step(now,age,true,policy,[&](const typename HandshakeLane<Frame>::Request &r){
   return choosePriority(priority_.snapshot())==TxPriority::Handshake&&hsAdmit(r);
  });});
 }
};
}
