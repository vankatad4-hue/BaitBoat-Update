#include "lora_link_policy.h"
#include <cassert>
using namespace flytec_rtos;
static void rollingLedger(){
 LoRaRollingLedger ledger;
 // A charge near a minute boundary cannot expire at the next hour boundary.
 assert(ledger.reserve(59999,25,50)&&ledger.finish(60010));
 assert(ledger.reserve(60011,25,50)&&ledger.finish(60050));
 assert(ledger.usedMs(3600000)==50);
 assert(!ledger.reserve(3660049,25,50));
 assert(ledger.usedMs(3660050)==0&&ledger.reserve(3660050,25,50));
 // An uncertain write stays active even after more than an hour.
 assert(ledger.usedMs(7260051)==25&&!ledger.reserve(7260051,25,50));
 LoRaRollingLedger wrap;const uint32_t start=UINT32_MAX-20;
 assert(wrap.reserve(start,25,50)&&wrap.finish(start+40));
 assert(wrap.usedMs(start+3600039)==25&&wrap.usedMs(start+3600040)==0);
 // Distinct minute buckets remain conservative and never overbook cap.
 LoRaRollingLedger buckets;
 for(uint32_t n=0;n<60;++n){assert(buckets.reserve(n*60000,25,1500));assert(buckets.finish(n*60000+100));}
 assert(buckets.usedMs(3599999)==1500&&!buckets.reserve(3599999,25,1500));
 assert(buckets.reserve(3600100,25,1500)&&buckets.finish(3600200));
 assert(!buckets.reserve(3600201,0,1500)&&!buckets.reserve(3600201,1,3600001));
 // A failed configuration may not consume a valid quiet-history proof.
 LoRaQuietRestart proof;for(uint32_t n=0;n<=3600000;n+=1000)proof.observe(n,true);
 LoRaAirtimePolicy policy;LoRaAirtimeProfile p;
 assert(!policy.configureAfterQuiet(p,3600000,proof)&&proof.ready(3600000));
 p.commissioned=true;assert(policy.configureAfterQuiet(p,3600000,proof)&&!proof.ready(3600000));
}
int main(){
 rollingLedger();
 using T=LoRaAirtimePolicy::Traffic;using D=LoRaAirtimePolicy::Decision;
 LoRaAirtimePolicy a;LoRaAirtimeProfile p;
 assert(!a.configure(p,0,true));assert(a.reserve(T::Emergency,0,true)==D::Uncommissioned);
 p.commissioned=true;p.hourlyMs=100;p.reserveMs=25;p.packetMs=25;
 assert(a.configure(p,0,true));assert(!a.configure(p,0,true));
 assert(a.reserve(T::Routine,0,false)==D::Busy);
 assert(a.reserve(T::Routine,0,true)==D::Allowed);
 assert(a.reserve(T::Emergency,1000,true)==D::Busy);assert(a.finish(100));
 assert(a.reserve(T::Routine,3099,true)==D::Busy);
 assert(a.reserve(T::Mission,600,true)==D::Allowed&&a.finish(700));
 assert(a.reserve(T::Mission,1200,true)==D::Allowed&&a.finish(1300));
 assert(a.reserve(T::Mission,1800,true)==D::DutyLimited);
 assert(a.reserve(T::Emergency,1800,true)==D::Allowed&&a.finish(1900));
 assert(a.reserve(T::Emergency,2400,true)==D::DutyLimited);
 assert(a.usedMs(3600000)==100); // conservative bucket expiry after last possible end
 assert(a.usedMs(3601899)==100&&a.usedMs(3601900)==0);
 LoRaAirtimePolicy b;assert(b.configure(p,100,false));
 assert(b.reserve(T::Emergency,3600099,true)==D::Uncommissioned);
 assert(b.reserve(T::Emergency,3600100,true)==D::Allowed&&b.finish(3600200));
 LoRaAirtimePolicy wrap;uint32_t at=0xffffff00U;assert(wrap.configure(p,at,true));
 assert(wrap.reserve(T::Emergency,at,true)==D::Allowed&&wrap.finish(at+100));
 assert(wrap.reserve(T::Emergency,at+599,true)==D::Busy);
 assert(wrap.reserve(T::Emergency,at+600,true)==D::Allowed&&wrap.finish(at+700));
 assert(wrap.usedMs(at+3600699)==50&&wrap.usedMs(at+3600700)==0);
 LoRaAirtimePolicy invalid;p.reserveMs=0;assert(!invalid.configure(p,0,true));
}
