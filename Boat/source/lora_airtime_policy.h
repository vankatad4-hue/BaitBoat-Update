#pragma once
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
