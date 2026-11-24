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
   retireApplication();return false;
  }
  return true;
 }
 bool tick(uint32_t now,bool primaryFault=false,bool backupFault=false){
  if(!owner_.worker()||retired_)return false;
  if((primaryFault||handoff_.fault(PhyLink::Primary))&&
     (backupFault||handoff_.fault(PhyLink::Backup))){
   requestStop();return false;
  }
  if(!runtime_.tick(now)){retireApplication();return false;}
  return true;
 }
 bool permit()const{return owner_.worker()&&!retired_&&runtime_.permit();}
 void requestStop(){
  if(!owner_.worker()||retired_)return;
  retireApplication();runtime_.requestStop();
 }
 bool exited()const{return runtime_.exited();}
private:
 static void retireThunk(void* p){static_cast<RemoteExclusiveRuntime*>(p)->retireApplication();}
 void retireApplication(){
  if(retired_||!owner_.worker())return;
  retired_=true;
  revoke_(context_); // revoke TX/session and application authority first
  owner_.retire();
 }
 RemoteRadioOwner& owner_;RadioFrameHandoff& handoff_;Runtime& runtime_;
 Callback revoke_=nullptr;void* context_=nullptr;
 bool attempted_=false,retired_=false;
};
}
