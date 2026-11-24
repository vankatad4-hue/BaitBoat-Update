#pragma once
#include "remote_i2c_owner.h"
#include "radio_frame_handoff.h"
// R30H-111: dormant request/response dispatcher. No raw I2C addresses,
// pointers or arbitrary register writes cross cores. Owner executes a
// complete radio service step; callbacks must be bounded and non-reentrant.
// Initialize before producers start and destroy only after they join.
namespace flytec_rtos {
class RemoteI2cDispatcher {
public:
 enum class Operation:uint8_t { PollPrimary=1, PollBackup=2 };
 enum class Outcome:uint8_t { Completed, Failed, Expired, Invalid };
 struct Request {uint32_t ticket;uint32_t issuedMs;Operation op;};
 struct Result {uint32_t ticket;Operation op;Outcome outcome;};
 using Execute=bool(*)(void*,Operation);
 static constexpr uint32_t maxAgeMs=100;
 bool begin(){return remoteI2cAllowed()&&requests_.begin()&&results_.begin();}
 bool submit(const Request& r){
  if(fault()||!r.ticket||!valid(r.op))return false;
  return requests_.offer(r);
 }
 bool take(Result& r){return results_.take(r);}
 bool fault()const{return requests_.fault()||results_.fault();}
 // At most one complete request per call. Contention with a foreign task
 // never becomes a hardware operation. Age is measured in the common MCU clock.
 bool service(uint32_t now,Execute execute,void* ctx){
  if(!remoteI2cAllowed()||fault()||!execute)return false;
  Request r{};
  if(!requests_.take(r))return true;
  Outcome outcome=Outcome::Invalid;
  if(valid(r.op)){
   if(uint32_t(now-r.issuedMs)>=maxAgeMs)outcome=Outcome::Expired;
   else outcome=execute(ctx,r.op)?Outcome::Completed:Outcome::Failed;
  }
  return results_.offer(Result{r.ticket,r.op,outcome});
 }
private:
 static bool valid(Operation op){return op==Operation::PollPrimary||op==Operation::PollBackup;}
 FrameQueue<Request,4> requests_;
 FrameQueue<Result,4> results_;
};
}
