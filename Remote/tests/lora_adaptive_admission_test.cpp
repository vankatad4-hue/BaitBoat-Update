#include "lora_link_policy.h"
#include <cassert>
using namespace flytec_rtos;
struct Driver{
 enum class Admission{Accepted,Busy,DutyLimited,Invalid,Unavailable,Uncertain};
 enum class Outcome{Idle,Pending,LocalReady,Uncertain};
 Admission next=Admission::Accepted;Outcome outcome=Outcome::Idle;
 bool available=true,configured_=true,fault=false;unsigned calls=0;
 bool ready()const{return available&&configured_&&!fault&&!txPending();}
 bool configured()const{return configured_;}
 bool transportFault()const{return fault;}
 bool txPending()const{return outcome==Outcome::Pending;}
 Outcome txOutcome()const{return outcome;}
 Admission trySendFrame(int,uint32_t){++calls;if(next==Admission::Accepted)outcome=Outcome::Pending;return next;}
};
int main(){
 assert(!E22UartProfile::fastPermitted(true,true,true,false,true));
 assert(E22UartProfile::fastPermitted(true,true,true,true,true));
 assert(!E22UartProfile::frameWireMs(0));
 using Adapter=E22AdaptiveAdmission<Driver>;using R=Adapter::Result;using P=Adapter::Poll;using T=Adapter::Traffic;
 LoRaAirtimeProfile cfg;cfg.commissioned=true;cfg.hourlyMs=100;cfg.reserveMs=25;cfg.packetMs=25;
 LoRaAirtimePolicy policy;assert(policy.configure(cfg,0,true));Driver driver;
 Adapter denied(driver,policy,20,false);assert(denied.start(0,T::Emergency,0)==R::Denied&&driver.calls==0);
 Adapter a(driver,policy,20,true);driver.available=false;
 assert(a.start(0,T::Routine,0)==R::Busy&&policy.usedMs(0)==0&&driver.calls==0);
 driver.available=true;
 auto send=[&](T t,uint32_t at){assert(a.start(0,t,at)==R::Accepted);assert(a.poll(at)==P::Pending);
  driver.outcome=Driver::Outcome::LocalReady;assert(a.poll(at+10)==P::Pending);
  assert(a.poll(at+29)==P::Pending&&a.poll(at+30)==P::LocalComplete);};
 send(T::Routine,0);assert(a.start(0,T::Emergency,529)==R::Busy);
 send(T::Mission,530);send(T::Mission,1060);
 assert(a.start(0,T::Mission,1590)==R::DutyLimited&&driver.calls==3);
 send(T::Emergency,1590);assert(a.start(0,T::Emergency,2120)==R::DutyLimited&&driver.calls==4);
 LoRaAirtimePolicy p2;assert(p2.configure(cfg,0,true));Driver d2;Adapter b(d2,p2,20,true);
 d2.next=Driver::Admission::Busy;assert(b.start(0,T::Routine,0)==R::NoWrite&&p2.usedMs(0)==25);
 d2.next=Driver::Admission::Uncertain;assert(b.start(0,T::Emergency,500)==R::Fault);
 assert(p2.usedMs(3600501)==50);assert(b.poll(3600501)==P::Fault&&b.start(0,T::Emergency,3600501)==R::Fault);
 LoRaAirtimePolicy p3;assert(p3.configure(cfg,0,true));Driver d3;Adapter c(d3,p3,20,true);
 assert(c.start(0,T::Emergency,0)==R::Accepted);d3.fault=true;
 assert(c.poll(2000)==P::Fault&&p3.usedMs(3602000)==25);
}
