#pragma once
#ifndef FLYTEC_E22_ADAPTIVE_ADMISSION_SHARED_H
#define FLYTEC_E22_ADAPTIVE_ADMISSION_SHARED_H
#include "lora_airtime_policy.h"
namespace flytec_rtos {
// Dormant owner-only wrapper for an E22-compatible driver. One persistent
// policy per transmitting device; no direct driver bypass while active.
// Traffic comes from the authenticated application, not received RF bytes.
template<class Driver> class E22AdaptiveAdmission {
public:
 using Traffic=LoRaAirtimePolicy::Traffic;
 enum class Result:uint8_t{Accepted,Busy,DutyLimited,Denied,NoWrite,Fault};
 enum class Poll:uint8_t{Idle,Pending,LocalComplete,Fault};
 // tailMs must bound possible remaining RF airtime AFTER local readiness.
 // Zero or unapproved evidence disables all transmissions through this seam.
 E22AdaptiveAdmission(Driver& d,LoRaAirtimePolicy& policy,uint32_t tailMs,bool approved):
  driver_(d),policy_(policy),tailMs_(tailMs),approved_(approved&&tailMs&&tailMs<60000U){}
 E22AdaptiveAdmission(const E22AdaptiveAdmission&)=delete;
 E22AdaptiveAdmission& operator=(const E22AdaptiveAdmission&)=delete;
 template<class Frame> Result start(const Frame& frame,Traffic traffic,uint32_t now){
  if(fault_||driver_.transportFault())return Result::Fault;
  if(!approved_)return Result::Denied;
  if(active_||!driver_.ready())return Result::Busy;
  const auto d=policy_.reserve(traffic,now,true);
  using D=LoRaAirtimePolicy::Decision;
  if(d==D::Busy)return Result::Busy;
  if(d==D::DutyLimited)return Result::DutyLimited;
  if(d!=D::Allowed)return Result::Denied;
  // Charge before possible write. Ambiguous attempts are never refunded.
  using A=typename Driver::Admission;
  const auto result=driver_.trySendFrame(frame,now);
  if(result==A::Accepted){active_=true;tailStarted_=false;return Result::Accepted;}
  if(result==A::Uncertain||driver_.transportFault()){fault_=true;return Result::Fault;}
  // Known pre-write rejection: retain conservative charge; owner must retire
  // the request rather than repeatedly burn budget on the same request.
  if(!policy_.finish(now)){fault_=true;return Result::Fault;}
  return Result::NoWrite;
 }
 // Owner updates the driver first. Local completion is not an RF ACK.
 Poll poll(uint32_t now){
  if(fault_||driver_.transportFault()){fault_=true;return Poll::Fault;}
  if(!active_)return Poll::Idle;
  if(!driver_.configured()){fault_=true;return Poll::Fault;}
  if(driver_.txPending())return Poll::Pending;
  using O=decltype(driver_.txOutcome());
  if(driver_.txOutcome()!=O::LocalReady){fault_=true;return Poll::Fault;}
  if(!tailStarted_){tailStarted_=true;tailAt_=now;}
  if(uint32_t(now-tailAt_)<tailMs_)return Poll::Pending;
  if(!policy_.finish(now)){fault_=true;return Poll::Fault;}
  active_=false;tailStarted_=false;return Poll::LocalComplete;
 }
private:
 Driver& driver_;LoRaAirtimePolicy& policy_;const uint32_t tailMs_;const bool approved_;
 uint32_t tailAt_=0;bool active_=false,tailStarted_=false,fault_=false;
};
}
#endif
