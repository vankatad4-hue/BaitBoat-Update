#pragma once
#include "remote_i2c_owner.h"
#include "radio_frame_handoff.h"
// Dormant R30H-112: per-link typed poll request/response queues.
// Single producer and consumer per direction; only bound owner executes.
// No arbitrary I2C address, pointer or register writes cross cores.
// Initialize before producers start; destroy only after they join.
namespace flytec_rtos {
class RemoteI2cDispatcher {
public:
 enum class Operation:uint8_t { PollPrimary=1, PollBackup=2 };
 enum class Outcome:uint8_t { Completed, Failed, Expired, Invalid };
 struct Request {uint32_t ticket;uint32_t issuedMs;Operation op;};
 struct Result {uint32_t ticket;Operation op;Outcome outcome;};
 using Execute=bool(*)(void*,Operation);
 static constexpr uint32_t maxAgeMs=100;
 bool begin(){return remoteI2cAllowed()&&primary_.begin()&&backup_.begin();}
 bool submit(const Request& r){
  auto* q=channel(r.op);
  return q&&r.ticket&&!q->fault()&&q->requests.offer(r);
 }
 bool take(Operation op,Result& r){
  auto* q=channel(op);return q&&q->results.take(r);
 }
 bool fault(Operation op)const{
  const auto* q=channel(op);return !q||q->fault();
 }
 bool fault()const{return primary_.fault()||backup_.fault();}
 // At most one complete operation per call. Alternate after a serviced
 // request, skip empty or quarantined lanes. A blocked bus is still shared.
 bool service(uint32_t now,Execute execute,void* ctx){
  if(!remoteI2cAllowed()||!execute)return false;
  if(primary_.fault()&&backup_.fault())return false;
  auto first=nextBackup_?Operation::PollBackup:Operation::PollPrimary;
  auto second=nextBackup_?Operation::PollPrimary:Operation::PollBackup;
  Request r{};
  auto* q=channel(first);
  if(q->fault()||!q->requests.take(r)){
   q=channel(second);
   if(q->fault()||!q->requests.take(r))return true;
   first=second;
  }
  nextBackup_=first==Operation::PollPrimary;
  Outcome outcome=Outcome::Invalid;
  if(r.op==first){
   if(uint32_t(now-r.issuedMs)>=maxAgeMs)outcome=Outcome::Expired;
   else outcome=execute(ctx,r.op)?Outcome::Completed:Outcome::Failed;
  }
  return q->results.offer(Result{r.ticket,first,outcome});
 }
private:
 struct Channel {
  FrameQueue<Request,4> requests;
  FrameQueue<Result,4> results;
  bool begin(){return requests.begin()&&results.begin();}
  bool fault()const{return requests.fault()||results.fault();}
 };
 Channel* channel(Operation op){return op==Operation::PollPrimary?&primary_:op==Operation::PollBackup?&backup_:nullptr;}
 const Channel* channel(Operation op)const{return op==Operation::PollPrimary?&primary_:op==Operation::PollBackup?&backup_:nullptr;}
 Channel primary_,backup_;
 bool nextBackup_=false; // owner only
};
}
