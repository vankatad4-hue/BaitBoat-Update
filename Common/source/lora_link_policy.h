#pragma once
// Consolidated LoRa timing, airtime and driver-admission policy.
#ifndef FLYTEC_LORA_AIRTIME_POLICY_SHARED_H
#define FLYTEC_LORA_AIRTIME_POLICY_SHARED_H
#include <stdint.h>
namespace flytec_rtos {
// Experimental owner-only policy. No physical radio enabling or legal approval.
// Charge before any possible UART/RF write; do not refund ambiguous attempts.
struct LoRaAirtimeProfile {
 static constexpr uint32_t BaselineHour=36000U,BaselinePacket=25U,BaselineGap=3000U;
 static constexpr uint32_t CandidateReserve=3600U,CandidateUrgentGap=500U;
 uint32_t hourlyMs=BaselineHour, reserveMs=CandidateReserve, packetMs=BaselinePacket;
 uint32_t normalGapMs=BaselineGap, urgentGapMs=CandidateUrgentGap;
 bool commissioned=false;
};
class LoRaAirtimePolicy {
public:
 enum class Traffic:uint8_t{Routine,Mission,Emergency};
 enum class Decision:uint8_t{Allowed,Uncommissioned,Invalid,Busy,DutyLimited};
 static constexpr uint32_t Window=3600000U, Bucket=60000U;
 // A fresh object needs trusted prior-hour history or a full quiet hour.
 // No in-place reconfiguration can erase prior charges.
 bool configure(const LoRaAirtimeProfile& p,uint32_t now,bool priorHourQuiet){
  if(configured_||!p.commissioned||!p.hourlyMs||p.hourlyMs>Window||
     !p.packetMs||p.packetMs>p.reserveMs||p.reserveMs>=p.hourlyMs||
     p.packetMs>p.hourlyMs-p.reserveMs||!p.urgentGapMs||
     p.normalGapMs<p.urgentGapMs||p.normalGapMs>=Window)return false;
  profile_=p;configured_=true;bootAt_=now;quiet_=priorHourQuiet;return true;
 }
 Decision inspect(Traffic t,uint32_t now,bool phyReady){
  if(!configured_||(!quiet_&&uint32_t(now-bootAt_)<Window))return Decision::Uncommissioned;
  quiet_=true;
  if(t!=Traffic::Routine&&t!=Traffic::Mission&&t!=Traffic::Emergency)return Decision::Invalid;
  if(!phyReady||outstanding_)return Decision::Busy;
  const uint32_t gap=t==Traffic::Routine?profile_.normalGapMs:profile_.urgentGapMs;
  if(sent_&&uint32_t(now-lastPossible_)<gap)return Decision::Busy;
  const uint32_t cap=t==Traffic::Emergency?profile_.hourlyMs:profile_.hourlyMs-profile_.reserveMs;
  const uint32_t used=usedMs(now);
  if(used>cap||profile_.packetMs>cap-used)return Decision::DutyLimited;
  return Decision::Allowed;
 }
 Decision reserve(Traffic t,uint32_t now,bool phyReady){
  const auto decision=inspect(t,now,phyReady);if(decision!=Decision::Allowed)return decision;
  Entry* slot=nullptr;
  for(auto& e:entries_)if(e.valid&&uint32_t(now-e.first)<Bucket){slot=&e;break;}
  if(!slot)for(auto& e:entries_)if(!e.valid){slot=&e;break;}
  if(!slot)return Decision::DutyLimited;
  if(!slot->valid){slot->valid=true;slot->first=now;slot->ms=0;}
  slot->last=now;slot->ms+=profile_.packetMs;active_=slot;outstanding_=true;sent_=true;lastPossible_=now;
  return Decision::Allowed;
 }
 // Call only after local completion or a bounded uncertain final RF end.
 // Keep the charge alive from this conservative end timestamp, not admission.
 bool finish(uint32_t now){
  if(!outstanding_||!active_)return false;
  active_->last=now;lastPossible_=now;active_=nullptr;outstanding_=false;return true;
 }
 uint32_t usedMs(uint32_t now){
  uint32_t total=0;
  for(auto& e:entries_){
   if(e.valid && &e!=active_&&uint32_t(now-e.last)>=Window)e.valid=false;
   if(e.valid)total+=e.ms;
  }
  return total;
 }
 LoRaAirtimePolicy()=default;
 LoRaAirtimePolicy(const LoRaAirtimePolicy&)=delete;
 LoRaAirtimePolicy& operator=(const LoRaAirtimePolicy&)=delete;
private:
 struct Entry{uint32_t first=0,last=0,ms=0;bool valid=false;};
 Entry entries_[62]{};Entry* active_=nullptr;LoRaAirtimeProfile profile_{};
 uint32_t bootAt_=0,lastPossible_=0;bool configured_=false,quiet_=false,sent_=false,outstanding_=false;
};
}

#endif // FLYTEC_LORA_AIRTIME_POLICY_SHARED_H

#ifndef FLYTEC_E22_ADAPTIVE_ADMISSION_SHARED_H
#define FLYTEC_E22_ADAPTIVE_ADMISSION_SHARED_H
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
 bool belongsTo(const Driver& d)const{return &driver_==&d;}
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

#ifndef FLYTEC_E22_UART_PROFILE_SHARED_H
#define FLYTEC_E22_UART_PROFILE_SHARED_H
#include <stdint.h>
namespace flytec_rtos {
// E22-900T22S candidate only; confirm module revision before register writes.
// Setup mode is 9600 8N1 even if transparent runtime uses faster UART.
struct E22UartProfile {
 static constexpr uint32_t ConfigBaud=9600U, BaselineBaud=9600U, CandidateBaud=115200U;
 static constexpr uint8_t BaselineReg0=0x67U,CandidateReg0=0xe7U;
 static constexpr uint32_t frameWireMs(uint32_t baud){return baud==BaselineBaud||baud==CandidateBaud?(320000U+baud-1U)/baud:0U;}
 // All prerequisites are independently supplied by the commissioning owner.
 static constexpr bool fastPermitted(bool exactModule,bool hostBaudVerified,bool registerReadback,bool normalModeVerified,bool rfProfileApproved){
  return exactModule&&hostBaudVerified&&registerReadback&&normalModeVerified&&rfProfileApproved;
 }
};
static_assert(E22UartProfile::frameWireMs(9600)==34,"32-byte UART baseline");
static_assert(E22UartProfile::frameWireMs(115200)==3,"32-byte UART candidate");
static_assert((E22UartProfile::BaselineReg0&0x1f)==(E22UartProfile::CandidateReg0&0x1f),"Only UART baud bits change");
}
#endif
