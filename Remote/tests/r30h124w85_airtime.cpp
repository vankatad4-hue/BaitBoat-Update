#include "lora_airtime_policy.h"
#include <cassert>
using namespace flytec_rtos;
int main(){
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
