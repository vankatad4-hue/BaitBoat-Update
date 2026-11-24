#pragma once
#include "remote_i2c_dispatcher.h"
#include <atomic>
// Dormant R30H-113. Sole producer/consumer on Core 0; owner on Core 1.
// One outstanding request per lane. A timeout quarantines that lane;
// late results cannot re-enable it. No implicit retry or ticket reuse.
// Status is published atomically; poll completion is not RF connectivity.
namespace flytec_rtos {
class RemoteI2cRequestScheduler {
public:
 using D=RemoteI2cDispatcher;
 enum class State:uint8_t { Idle, Waiting, Completed, Fault };
 explicit RemoteI2cRequestScheduler(D& d):dispatcher_(d){}
 RemoteI2cRequestScheduler(const RemoteI2cRequestScheduler&)=delete;
 RemoteI2cRequestScheduler& operator=(const RemoteI2cRequestScheduler&)=delete;
 static constexpr uint32_t responseTimeoutMs=200; // provisional diagnostic limit
 void tick(uint32_t now){
  step(primary_,D::Operation::PollPrimary,now);
  step(backup_,D::Operation::PollBackup,now);
 }
 State status(D::Operation op)const{
  if(op==D::Operation::PollPrimary)return primary_.published.load();
  if(op==D::Operation::PollBackup)return backup_.published.load();
  return State::Fault;
 }
private:
 struct Lane {
  uint32_t ticket=0,issued=0;
  bool pending=false;
  std::atomic<State> published{State::Idle};
 };
 void step(Lane& l,D::Operation op,uint32_t now){
  if(l.published.load()==State::Fault)return;
  if(dispatcher_.fault(op)){l.published.store(State::Fault);return;}
  D::Result r{};
  if(l.pending&&uint32_t(now-l.issued)>=responseTimeoutMs){
   l.published.store(State::Fault);return;
  }
  if(dispatcher_.take(op,r)){
   if(!l.pending||r.ticket!=l.ticket||r.op!=op||r.outcome!=D::Outcome::Completed){
    l.published.store(State::Fault);return;
   }
   l.pending=false;l.published.store(State::Completed);
   return; // no second request in this iteration
  }
  if(l.pending)return;
  if(l.ticket==UINT32_MAX){l.published.store(State::Fault);return;}
  ++l.ticket;
  if(!dispatcher_.submit({l.ticket,now,op})){l.published.store(State::Fault);return;}
  l.issued=now;l.pending=true;l.published.store(State::Waiting);
 }
 D& dispatcher_;
 Lane primary_,backup_;
};
}
