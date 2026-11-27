#pragma once
#include "remote_radio_owner.h"
#include "remote_worker_runtime.h"
#include "radio_frame_handoff.h"
// Exclusive one-shot composition for the isolated mock or a later commissioned
// application. No hardware initialization or commissioning policy here.
// Owner and all queue/driver storage must survive until runtime.exited().
namespace flytec_rtos {
template<class Runtime> class RemoteExclusiveRuntime {
public:
 using Callback=void(*)(void*);
 using Initialize=bool(*)(void*);
 RemoteExclusiveRuntime(RemoteRadioOwner& owner,RadioFrameHandoff& handoff,Runtime& runtime):
  owner_(owner),handoff_(handoff),runtime_(runtime){}
 RemoteExclusiveRuntime(const RemoteExclusiveRuntime&)=delete;
 RemoteExclusiveRuntime& operator=(const RemoteExclusiveRuntime&)=delete;
 bool begin(bool authorized,Initialize initialize,Callback revoke,void* ctx){
  if(!remoteI2cAllowed()||attempted_||!authorized||!initialize||!revoke)return false;
  attempted_=true;
  if(!owner_.claimWorker(true))return false;
  revoke_=revoke;context_=ctx;
  if(!runtime_.bindRetirement(&retireThunk,this)||!handoff_.begin()||
     !initialize(ctx)||!runtime_.begin(true,true)){
   // A bind failure can mean this runtime was already started elsewhere.
   // Revoke this application claim, then stop any worker that may be live.
   retireApplication();runtime_.requestStop();return false;
  }
  return true;
 }
 bool tick(uint32_t now,bool primaryFault=false,bool backupFault=false){
  if(!remoteI2cAllowed()||retired_)return false;
  if(!owner_.worker()){requestStop();return false;}
  if((primaryFault||handoff_.fault(PhyLink::Primary))&&
     (backupFault||handoff_.fault(PhyLink::Backup))){
   requestStop();return false;
  }
  if(!runtime_.tick(now)){retireApplication();return false;}
  return true;
 }
 bool permit()const{return owner_.worker()&&!retired_&&runtime_.permit();}
 void requestStop(){
  if(!remoteI2cAllowed()||!attempted_)return;
  retireApplication();runtime_.requestStop();
 }
 bool exited()const{return runtime_.exited();}
private:
 static void retireThunk(void* p){static_cast<RemoteExclusiveRuntime*>(p)->retireApplication();}
 void retireApplication(){
  if(retired_)return;
  retired_=true;
  // Revoke app TX/session state even if another path retired the owner first.
  // In the normal path this callback runs before the owner mode is retired.
  revoke_(context_);
  if(owner_.worker())owner_.retire();
 }
 RemoteRadioOwner& owner_;RadioFrameHandoff& handoff_;Runtime& runtime_;
 Callback revoke_=nullptr;void* context_=nullptr;
 bool attempted_=false,retired_=false;
};
}

