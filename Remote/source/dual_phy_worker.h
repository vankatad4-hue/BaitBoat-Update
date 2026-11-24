#pragma once
#include "dual_phy_poll.h"
#include "radio_worker.h"
#include "radio_watchdog.h"
// Dormant diagnostic wrapper. External owner must initialize drivers, handoff,
// and generation, and verify RF and bus gates before begin. Do not use while
// the legacy scheduler owns the drivers. No automatic restart after a fault.
// Caller must stop and join before destroying borrowed objects.
namespace flytec_rtos {
template<class Poller> class DualPhyWorker {
public:
 using Clock=uint32_t(*)();
 explicit DualPhyWorker(Poller& poll,Clock clock):poll_(poll),clock_(clock){}
 DualPhyWorker(const DualPhyWorker&)=delete;
 DualPhyWorker& operator=(const DualPhyWorker&)=delete;
 bool begin(bool commissioningAuthorized){
  if(!commissioningAuthorized||!clock_||started_||retired_)return false;
  retired_=true; // a failed launch is terminal for this instance
  if(!worker_.begin(&service,this))return false;
  started_=true;
  watchdog_.reset(clock_(),worker_.heartbeat());
  return true;
 }
 RadioWatchdog::State supervise(){
  if(!started_)return RadioWatchdog::State::Fault;
  return watchdog_.update(clock_(),worker_.heartbeat(),worker_.running(),worker_.wrongCore());
 }
 bool permit()const{return !stopping_&&started_&&worker_.running()&&!worker_.wrongCore()&&watchdog_.permit();}
 // Blocking join, only after bounded callback execution has been established.
 // Never call from an emergency actuator shutdown path.
 void requestStop(){stopping_=true;worker_.requestStop();}
 bool exited()const{return worker_.exited();}
 void stop(){requestStop();worker_.stop();}
private:
 static void service(void* ctx){
  auto& self=*static_cast<DualPhyWorker*>(ctx);
  self.poll_.tick(self.clock_());
 }
 Poller& poll_;Clock clock_;
 RadioWatchdog watchdog_;
 bool started_=false,retired_=false,stopping_=false;
 RadioWorker worker_; // destroyed first; joins before borrowed members used by service disappear
};
}
