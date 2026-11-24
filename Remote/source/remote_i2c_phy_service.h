#pragma once
#include "remote_i2c_dispatcher.h"
#include "tx_generation_gate.h"
// Dormant R30H-112. Core 1 owns both physical adapters. Core 0 may
// request service only, never call the physical driver. No legacy
// scheduler may use these same adapter instances concurrently.
namespace flytec_rtos {
template<class Primary,class Backup> class RemoteI2cPhyService {
public:
 RemoteI2cPhyService(RemoteI2cDispatcher& d,Primary& p,Backup& b,
                     RadioFrameHandoff& h,TxGenerationGate& g):
 dispatcher_(d),primary_(p),backup_(b),handoff_(h),gate_(g){}
 RemoteI2cPhyService(const RemoteI2cPhyService&)=delete;
 RemoteI2cPhyService& operator=(const RemoteI2cPhyService&)=delete;
 bool service(uint32_t now){
  Context context{this,now};
  return dispatcher_.service(now,&execute,&context);
 }
private:
 struct Context{RemoteI2cPhyService* self;uint32_t now;};
 static bool execute(void* ptr,RemoteI2cDispatcher::Operation op){
  auto& c=*static_cast<Context*>(ptr);auto& s=*c.self;
  if(op==RemoteI2cDispatcher::Operation::PollPrimary)
   return s.primary_.tick(s.handoff_,s.gate_,c.now);
  if(op==RemoteI2cDispatcher::Operation::PollBackup)
   return s.backup_.tick(s.handoff_,s.gate_,c.now);
  return false;
 }
 RemoteI2cDispatcher& dispatcher_;Primary& primary_;Backup& backup_;
 RadioFrameHandoff& handoff_;TxGenerationGate& gate_;
};
}
