#pragma once
#include "remote_i2c_owner.h"
#include "radio_watchdog.h"
namespace flytec_rtos {
// Experimental Core 1 coordinator. Core 0 requests polling only.
// All borrowed objects must outlive the worker, even after retirement.
template<class Worker,class Service> class RemoteWorkerRuntime {
public:
 enum class State:uint8_t {Idle,Starting,Running,Retired};
 using Retirement=void(*)(void*);
 RemoteWorkerRuntime(Worker& w,Service& s):worker_(w),service_(s){}
 RemoteWorkerRuntime(const RemoteWorkerRuntime&)=delete;
 RemoteWorkerRuntime& operator=(const RemoteWorkerRuntime&)=delete;
 // Optional application revocation callback, registered before launch.
 bool bindRetirement(Retirement fn,void* context){
  if(!remoteI2cAllowed()||state_!=State::Idle||!fn||retirement_)return false;
  retirement_=fn;context_=context;return true;
 }
 bool begin(bool commissioned,bool legacyAbsent){
  if(state_!=State::Idle||!commissioned||!legacyAbsent||!remoteI2cAllowed())return false;
  state_=State::Starting;
  if(!worker_.begin(true)){retire();return false;}
  return true;
 }
 bool tick(uint32_t now){
  if(!remoteI2cAllowed()||(state_!=State::Starting&&state_!=State::Running))return false;
  const auto health=worker_.supervise();
  if(health==RadioWatchdog::State::Fault){retire();return false;}
  if(!service_.service(now)){retire();return false;}
  if(health==RadioWatchdog::State::Healthy)state_=State::Running;
  return true;
 }
 void requestStop(){if(remoteI2cAllowed()&&state_!=State::Idle)retire();}
 bool exited()const{return worker_.exited();}
 bool permit()const{return state_==State::Running&&worker_.permit();}
 State state()const{return state_;}
private:
 void retire(){
  if(state_==State::Retired)return;
  state_=State::Retired;
  if(retirement_)retirement_(context_); // revoke application before worker stop
  worker_.requestStop();
 }
 Worker& worker_;Service& service_;
 State state_=State::Idle;
 Retirement retirement_=nullptr;void* context_=nullptr;
};
}
