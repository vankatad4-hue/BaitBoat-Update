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
// Immutable owner pacing hint; not airtime permission. The single PHY
// owner independently enforces the commissioned profile and RF-tail guard.
class LoRaOwnerTiming {
public:
 LoRaOwnerTiming()=default;
 static LoRaOwnerTiming fromProfile(const LoRaAirtimeProfile& p,bool adaptiveBound){
  LoRaOwnerTiming t;
  if(adaptiveBound&&p.commissioned&&p.normalGapMs==LoRaAirtimeProfile::BaselineGap&&
     p.urgentGapMs>=LoRaAirtimeProfile::CandidateUrgentGap&&p.urgentGapMs<=p.normalGapMs&&
     p.hourlyMs&&p.hourlyMs<=3600000U&&p.packetMs&&p.packetMs<=p.reserveMs&&
     p.reserveMs<p.hourlyMs&&p.packetMs<=p.hourlyMs-p.reserveMs)t.urgent_=p.urgentGapMs;
  return t;
 }
 uint32_t urgentGap()const{return urgent_;}
private:
 uint32_t urgent_=LoRaAirtimeProfile::BaselineGap;
};
// Conservative cold-start alternative to persisted airtime history. The owner
// must prove uninterrupted TX isolation (including RF tail) between samples;
// a missing sample or loss of that evidence restarts the full quiet hour.
class LoRaQuietRestart {
public:
 static constexpr uint32_t QuietMs=3600000U, MaxSampleGap=1000U;
 LoRaQuietRestart()=default;
 LoRaQuietRestart(const LoRaQuietRestart&)=delete;
 LoRaQuietRestart& operator=(const LoRaQuietRestart&)=delete;
 // Call from the exclusive PHY owner at least every MaxSampleGap. A missed
 // sample or failed isolation observation restarts the complete quiet hour.
 void observe(uint32_t now,bool continuousIsolation){
  if(!continuousIsolation){tracking_=false;return;}
  if(!tracking_||uint32_t(now-last_)>MaxSampleGap){tracking_=true;since_=now;}
  last_=now;
 }
 bool ready(uint32_t now)const{return tracking_&&uint32_t(now-last_)<=MaxSampleGap&&uint32_t(last_-since_)>=QuietMs;}
 bool consume(uint32_t now){if(!ready(now))return false;tracking_=false;return true;}
private:
 uint32_t since_=0,last_=0;bool tracking_=false;
};
// Shared conservative rolling ledger for one transmitter. Charges are
// grouped for at most 60 s from the first admission, and expire only a full
// hour after the most recent possible end in that group. Never age out an
// active/uncertain charge. All callers are on the exclusive radio owner.
class LoRaRollingLedger {
public:
 static constexpr uint32_t Window=3600000U,Bucket=60000U;
 LoRaRollingLedger()=default;
 LoRaRollingLedger(const LoRaRollingLedger&)=delete;
 LoRaRollingLedger& operator=(const LoRaRollingLedger&)=delete;
 bool pending()const{return active_!=nullptr;}
 bool hasCharges()const{for(const auto& e:entries_)if(e.valid&&e.ms)return true;return false;}
 uint32_t usedMs(uint32_t now){
  uint32_t total=0;
  for(auto& e:entries_){
   if(e.valid && &e!=active_&&uint32_t(now-e.last)>=Window)e.valid=false;
   if(e.valid)total+=e.ms;
  }
  return total;
 }
 bool reserve(uint32_t now,uint32_t charge,uint32_t cap){
  if(active_||!charge||cap>Window||charge>cap)return false;
  const uint32_t used=usedMs(now);
  if(used>cap||charge>cap-used)return false;
  Entry* slot=nullptr;
  for(auto& e:entries_)if(e.valid&&uint32_t(now-e.first)<Bucket){slot=&e;break;}
  if(!slot)for(auto& e:entries_)if(!e.valid){slot=&e;break;}
  if(!slot)return false;
  if(!slot->valid){slot->valid=true;slot->first=now;slot->ms=0;}
  slot->last=now;slot->ms+=charge;active_=slot;return true;
 }
 bool finish(uint32_t now){
  if(!active_)return false;
  active_->last=now;active_=nullptr;return true;
 }
private:
 struct Entry{uint32_t first=0,last=0,ms=0;bool valid=false;};
 Entry entries_[62]{};Entry* active_=nullptr;
};
class LoRaAirtimePolicy {
public:
 enum class Traffic:uint8_t{Routine,Mission,Emergency};
 enum class Decision:uint8_t{Allowed,Uncommissioned,Invalid,Busy,DutyLimited};
 static constexpr uint32_t Window=LoRaRollingLedger::Window, Bucket=LoRaRollingLedger::Bucket;
 // A fresh object needs trusted prior-hour history or a full quiet hour.
 // No in-place reconfiguration can erase prior charges.
 bool configure(const LoRaAirtimeProfile& p,uint32_t now,bool priorHourQuiet){
  if(configured_||!p.commissioned||!p.hourlyMs||p.hourlyMs>Window||
     !p.packetMs||p.packetMs>p.reserveMs||p.reserveMs>=p.hourlyMs||
     p.packetMs>p.hourlyMs-p.reserveMs||!p.urgentGapMs||
     p.normalGapMs<p.urgentGapMs||p.normalGapMs>=Window)return false;
  profile_=p;configured_=true;bootAt_=now;quiet_=priorHourQuiet;return true;
 }
 bool configureAfterQuiet(const LoRaAirtimeProfile& p,uint32_t now,LoRaQuietRestart& proof){
  if(configured_||!proof.ready(now)||!configure(p,now,true))return false;
  if(!proof.consume(now)){configured_=false;quiet_=false;profile_=LoRaAirtimeProfile{};return false;}
  quietProofConsumed_=true;return true;
 }
 bool managedReady()const{return configured_&&quietProofConsumed_;}
 Decision inspect(Traffic t,uint32_t now,bool phyReady){
  if(!configured_||(!quiet_&&uint32_t(now-bootAt_)<Window))return Decision::Uncommissioned;
  quiet_=true;
  if(t!=Traffic::Routine&&t!=Traffic::Mission&&t!=Traffic::Emergency)return Decision::Invalid;
  if(!phyReady||ledger_.pending())return Decision::Busy;
  const uint32_t gap=t==Traffic::Routine?profile_.normalGapMs:profile_.urgentGapMs;
  if(sent_&&uint32_t(now-lastPossible_)<gap)return Decision::Busy;
  const uint32_t cap=t==Traffic::Emergency?profile_.hourlyMs:profile_.hourlyMs-profile_.reserveMs;
  const uint32_t used=usedMs(now);
  if(used>cap||profile_.packetMs>cap-used)return Decision::DutyLimited;
  return Decision::Allowed;
 }
 Decision reserve(Traffic t,uint32_t now,bool phyReady){
  const auto decision=inspect(t,now,phyReady);if(decision!=Decision::Allowed)return decision;
  const uint32_t cap=t==Traffic::Emergency?profile_.hourlyMs:profile_.hourlyMs-profile_.reserveMs;
  if(!ledger_.reserve(now,profile_.packetMs,cap))return Decision::DutyLimited;
  sent_=true;lastPossible_=now;
  return Decision::Allowed;
 }
 // Call only after local completion or a bounded uncertain final RF end.
 // Keep the charge alive from this conservative end timestamp, not admission.
 bool finish(uint32_t now){
  if(!ledger_.finish(now))return false;
  lastPossible_=now;return true;
 }
 uint32_t usedMs(uint32_t now){return ledger_.usedMs(now);}
 LoRaAirtimePolicy()=default;
 LoRaAirtimePolicy(const LoRaAirtimePolicy&)=delete;
 LoRaAirtimePolicy& operator=(const LoRaAirtimePolicy&)=delete;
private:
 LoRaRollingLedger ledger_;LoRaAirtimeProfile profile_{};
 uint32_t bootAt_=0,lastPossible_=0;bool configured_=false,quiet_=false,sent_=false,quietProofConsumed_=false;
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
 enum class Binding:uint8_t{UnmanagedDriver,Bound,Rejected};
 template<class D> static auto bindManaged(D& d,int)->decltype(d.bindManagedAdmission(),Binding()){
  return d.bindManagedAdmission()?Binding::Bound:Binding::Rejected;
 }
 template<class D> static Binding bindManaged(D&,long){return Binding::UnmanagedDriver;}
 template<class D,class Frame> static auto sendFrame(D& d,const Frame& f,uint32_t now,bool managed,int)
  ->decltype(d.trySendFrameManaged(f,now)){
  return managed?d.trySendFrameManaged(f,now):d.trySendFrame(f,now);
 }
 template<class D,class Frame> static auto sendFrame(D& d,const Frame& f,uint32_t now,bool,long)
  ->decltype(d.trySendFrame(f,now)){
  return d.trySendFrame(f,now);
 }
 template<class D> static auto finishManaged(D& d,int)->decltype(d.finishManagedAdmission(),void()){
  d.finishManagedAdmission();
 }
 template<class D> static void finishManaged(D&,long){}
public:
 using Traffic=LoRaAirtimePolicy::Traffic;
 enum class Result:uint8_t{Accepted,Busy,DutyLimited,Denied,NoWrite,Fault};
 enum class Poll:uint8_t{Idle,Pending,LocalComplete,Fault};
 // tailMs must bound possible remaining RF airtime AFTER local readiness.
 // Zero or unapproved evidence disables all transmissions through this seam.
 E22AdaptiveAdmission(Driver& d,LoRaAirtimePolicy& policy,uint32_t tailMs,bool approved):
  driver_(d),policy_(policy),tailMs_(tailMs),approved_(approved&&tailMs&&tailMs<60000U),
  binding_(bindManaged(d,0)){}
 bool belongsTo(const Driver& d)const{return &driver_==&d;}
 E22AdaptiveAdmission(const E22AdaptiveAdmission&)=delete;
 E22AdaptiveAdmission& operator=(const E22AdaptiveAdmission&)=delete;
 template<class Frame> Result start(const Frame& frame,Traffic traffic,uint32_t now){
  if(fault_||driver_.transportFault())return Result::Fault;
  if(!approved_||binding_==Binding::Rejected||
     (binding_==Binding::Bound&&!policy_.managedReady()))return Result::Denied;
  if(active_||!driver_.ready())return Result::Busy;
  const auto d=policy_.reserve(traffic,now,true);
  using D=LoRaAirtimePolicy::Decision;
  if(d==D::Busy)return Result::Busy;
  if(d==D::DutyLimited)return Result::DutyLimited;
  if(d!=D::Allowed)return Result::Denied;
  // Charge before possible write. Ambiguous attempts are never refunded.
  using A=typename Driver::Admission;
  const auto result=sendFrame(driver_,frame,now,binding_==Binding::Bound,0);
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
  if(binding_==Binding::Bound)finishManaged(driver_,0);
  active_=false;tailStarted_=false;return Poll::LocalComplete;
 }
private:
 Driver& driver_;LoRaAirtimePolicy& policy_;const uint32_t tailMs_;const bool approved_;const Binding binding_;
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

#ifndef FLYTEC_E22_UART_TRANSITION_SHARED_H
#define FLYTEC_E22_UART_TRANSITION_SHARED_H
// Commissioning-only state machine. The port must implement bounded operations
// and prove isolation, exclusive ownership and complete UART drain. No API in
// this class grants application TX permission, including on successful rollback.
namespace flytec_rtos {
struct E22CommissioningPermit {
 bool exactModule=false,offlineOwner=false,rfProfileApproved=false,hostRatesVerified=false;
 bool isolatedAndDrained=false;
 bool valid()const{return exactModule&&offlineOwner&&rfProfileApproved&&hostRatesVerified&&isolatedAndDrained;}
};
// Exact integer divisor only, for audited crystal and divide-by-one MCR.
inline uint16_t sc16CommissioningDivisor(uint32_t crystal,uint32_t baud){
 if(baud!=E22UartProfile::ConfigBaud&&baud!=E22UartProfile::CandidateBaud)return 0;
 const uint32_t den=16U*baud;
 if(!crystal||crystal%den||crystal/den>65535U)return 0;
 return uint16_t(crystal/den);
}
// Offline commissioning host primitives. They never send an RF frame.
using E22IsolationProbe=bool(*)(void*);
inline bool uartBaudWithinOnePercent(uint32_t actual,uint32_t requested){
 if(requested!=E22UartProfile::ConfigBaud&&requested!=E22UartProfile::CandidateBaud)return false;
 const uint32_t difference=actual>requested?actual-requested:requested-actual;
 return difference<=requested/100U;
}
// NXP SC16IS750: FIFO free slots alone do not establish transmitter idle.
// Require normal register bank, TEMT, empty RX and no receive error.
template<class Read> bool sc16CommissioningDrained(Read read){
 uint8_t lcr=0,tx=0,lsr=0,rx=0;
 return read(3,lcr)&&lcr==0x03&&read(8,tx)&&tx==64&&read(5,lsr)&&
        (lsr&0x40U)&&(lsr&0x9fU)==0&&read(9,rx)&&rx==0;
}
// Recovery attachment is read-only. Reject an unknown bank, prescaler,
// interrupt mode or GPIO configuration; do not repair it by guessing.
template<class Read> bool sc16OfflineRecoveryReady(Read read){
 uint8_t mcr=0,ier=0,dir=0,state=0;
 return sc16CommissioningDrained(read)&&read(4,mcr)&&mcr==0&&read(1,ier)&&ier==0&&
        read(10,dir)&&dir==0x0bU&&read(11,state)&&(state&0x04U)&&
        ((state&0x0bU)==0x08U||(state&0x0bU)==0x0aU);
}
inline bool e22KnownProfile(const uint8_t* r,unsigned size){
 if(!r||size!=12||(r[6]!=E22UartProfile::BaselineReg0&&r[6]!=E22UartProfile::CandidateReg0))return false;
 const uint8_t expected[]={0xc1,0,9,0,0,0,r[6],0xc3,0x12,0,0,0};
 for(unsigned i=0;i<sizeof(expected);++i)if(r[i]!=expected[i])return false;
 return true;
}
template<class Read,class Write> bool sc16CommissioningSetBaud(Read read,Write write,uint32_t crystal,uint32_t baud){
 const uint16_t divisor=sc16CommissioningDivisor(crystal,baud);
 uint8_t mcr=0,ier=0,lcr=0,dll=0,dlh=0;
 if(!divisor||!sc16CommissioningDrained(read)||!read(4,mcr)||mcr!=0||!read(1,ier)||ier!=0)return false;
 // Verify DLAB before writing aliased THR/IER addresses. Never blindly write.
 const bool bank=write(3,0x83)&&read(3,lcr)&&lcr==0x83;
 const bool changed=bank&&write(0,uint8_t(divisor))&&write(1,uint8_t(divisor>>8))&&
                    read(0,dll)&&read(1,dlh)&&dll==uint8_t(divisor)&&dlh==uint8_t(divisor>>8);
 // Even a failed DLAB write is ambiguous: attempt known 8N1 restoration.
 // On any failure the caller stays locked; no application TX is allowed.
 const bool restored=write(3,0x03)&&read(3,lcr)&&lcr==0x03;
 return changed&&restored&&read(4,mcr)&&mcr==0&&read(1,ier)&&ier==0&&sc16CommissioningDrained(read);
}
// Only known offline configuration commands may reach the UART, never normal
// RF payloads. C2 is volatile and changes only REG0's baud bits.
inline bool e22CommissioningCommand(const uint8_t* cmd,unsigned size,unsigned responseSize){
 if(!cmd)return false;
 if(size==3)return responseSize==12&&cmd[0]==0xc1&&cmd[1]==0&&cmd[2]==9;
 return size==4&&responseSize==4&&cmd[0]==0xc2&&cmd[1]==3&&cmd[2]==1&&
        (cmd[3]==E22UartProfile::BaselineReg0||cmd[3]==E22UartProfile::CandidateReg0);
}
// Offline fixture must inject a fresh, caller-selected token only after the
// receive arm. Callback may not use this endpoint to transmit. No RF approval.
using E22FixtureArm=bool(*)(void*,uint32_t,const uint8_t*,unsigned,uint32_t);
// Host operations are private to this friend binding. Clock must be monotonic;
// the iteration cap also fails closed if the clock stops. No cached RX drain.
template<class Host> class E22OfflinePort {
public:
 E22OfflinePort(Host& host,uint32_t crystal,E22FixtureArm arm,void* context,const uint8_t (&token)[8]):
  host_(host),crystal_(crystal),arm_(arm),context_(context){for(unsigned i=0;i<8;++i)token_[i]=token[i];}
 E22OfflinePort(const E22OfflinePort&)=delete;
 E22OfflinePort& operator=(const E22OfflinePort&)=delete;
 bool lockedAndIsolated()const{return host_.commissioningAllowed();}
 bool modeVerified(bool config,uint32_t limit){
  if(!valid(limit)||!host_.commissioningModeVerified(config,limit))return fail();
  config_=config;modeKnown_=true;recognizedProfile_=false;return true;
 }
 bool baudVerified(uint32_t baud,uint32_t limit){
  if(!valid(limit)||!modeKnown_||!config_||!host_.commissioningHostBaud(crystal_,baud,limit))return fail();
  baud_=baud;recognizedProfile_=false;return true;
 }
 bool exchangeExact(const uint8_t* cmd,unsigned n,uint8_t* out,unsigned size,uint32_t limit){
  if(!valid(limit)||!config_||!modeKnown_||baud_!=9600||!out||!e22CommissioningCommand(cmd,n,size))return fail();
  // Even an offline caller must read a recognized complete profile before
  // a volatile write. Consume the proof before the possible write.
  if(n==4&&!recognizedProfile_)return fail();
  recognizedProfile_=false;
  const uint32_t start=host_.commissioningNow();
  unsigned count=0;
  if(!host_.commissioningModeMatches(true)||!host_.commissioningTxIdle()||
     !host_.commissioningRxCount(count)||count||!within(start,limit)||
     !host_.commissioningConfigWrite(cmd,n))return fail();
  if(!collect(out,size,start,limit))return fail();
  if(n==3){if(!e22KnownProfile(out,size))return fail();recognizedProfile_=true;}
  else if(out[0]!=0xc1||out[1]!=3||out[2]!=1||out[3]!=cmd[3])return fail();
  return true;
 }
 bool receiveFixtureVerified(uint32_t baud,uint32_t limit){
  if(!valid(limit)||!modeKnown_||config_||baud_!=baud||!arm_||fixtureUsed_)return fail();
  const uint32_t start=host_.commissioningNow();unsigned count=0;uint8_t data[8]{};
  uint8_t nonzero=0;for(auto b:token_)nonzero|=b;
  if(!nonzero||!host_.commissioningModeMatches(false)||!host_.commissioningTxIdle()||
     !host_.commissioningRxCount(count)||count||!within(start,limit))return fail();
  fixtureUsed_=true;
  if(!arm_(context_,baud,token_,8,limit)||!collect(data,8,start,limit))return fail();
  for(unsigned i=0;i<8;++i)if(data[i]!=token_[i])return fail();
  return true;
 }
private:
 bool valid(uint32_t limit)const{return lockedAndIsolated()&&limit>20U&&limit<=500U;}
 bool within(uint32_t start,uint32_t limit){return lockedAndIsolated()&&uint32_t(host_.commissioningNow()-start)<limit;}
 bool fail(){host_.commissioningLatchFault();return false;}
 bool collect(uint8_t* out,unsigned size,uint32_t start,uint32_t limit){
  unsigned received=0;uint32_t quietAt=0;bool quiet=false;
  for(unsigned polls=0;polls<limit+1U&&within(start,limit);++polls){
   unsigned count=0;
   if(!host_.commissioningModeMatches(config_)||!host_.commissioningRxCount(count)||count>size-received)return false;
   while(count--){if(!within(start,limit)||!host_.commissioningReadByte(out[received]))return false;++received;}
   if(received==size&&host_.commissioningTxIdle()){
    if(!quiet){quiet=true;quietAt=host_.commissioningNow();}
    if(uint32_t(host_.commissioningNow()-quietAt)>=10U)return within(start,limit);
   }else quiet=false;
   host_.commissioningWait();
  }
  return false;
 }
 Host& host_;const uint32_t crystal_;E22FixtureArm arm_;void* context_;uint8_t token_[8]{};
 uint32_t baud_=0;bool config_=false,modeKnown_=false,fixtureUsed_=false,recognizedProfile_=false;
};
class E22UartTransition {
public:
 enum class State:uint8_t{Idle,ConfigMode,ConfigBaud,ReadBefore,WriteTemporary,ReadAfter,RuntimeBaud,NormalMode,VerifyRx,VerifiedLocked,FaultLocked};
 static constexpr uint32_t StepLimit=500U,TotalLimit=5000U;
 E22UartTransition()=default;
 E22UartTransition(const E22UartTransition&)=delete;
 E22UartTransition& operator=(const E22UartTransition&)=delete;
 State state()const{return state_;}
 uint32_t verifiedBaud()const{return state_==State::VerifiedLocked?targetBaud_:0U;}
 // Explicit offline recovery is allowed after a failed transition or reboot.
 // It never writes a blind fallback: both baseline and candidate full profiles
 // must be recognized by the independent pre-write readback.
 template<class Port> bool begin(Port& p,const E22CommissioningPermit& permit,uint32_t now,bool recoverBaseline=false){
  if(state_!=State::Idle&&(!recoverBaseline||(state_!=State::FaultLocked&&state_!=State::VerifiedLocked)))return false;
  if(!permit.valid()||!p.lockedAndIsolated())return false;
  targetBaud_=recoverBaseline?E22UartProfile::BaselineBaud:E22UartProfile::CandidateBaud;
  port_=&p;recovering_=recoverBaseline;started_=now;state_=State::ConfigMode;return true;
 }
 // One bounded commissioning operation per call; never run from flight loop.
 // exchangeExact must verify complete response length, drained transmitter,
 // absence of extra bytes, and a fresh bounded reply (not a cached echo).
 template<class Port> bool step(Port& p,uint32_t now){
  if(state_==State::Idle||state_==State::FaultLocked||state_==State::VerifiedLocked)return false;
  if(port_!=&p||uint32_t(now-started_)>=TotalLimit||!p.lockedAndIsolated())return fail();
  const uint8_t target=targetBaud_==E22UartProfile::CandidateBaud?E22UartProfile::CandidateReg0:E22UartProfile::BaselineReg0;
  switch(state_){
   case State::ConfigMode:
    if(!p.modeVerified(true,StepLimit))return fail();
    state_=State::ConfigBaud;break;
   case State::ConfigBaud:
    if(!p.baudVerified(E22UartProfile::ConfigBaud,StepLimit))return fail();
    state_=State::ReadBefore;break;
   case State::ReadBefore:{
    uint8_t response[12]{};
    if(!readProfile(p,response)||(!matches(response,E22UartProfile::BaselineReg0)&&
       !(recovering_&&matches(response,E22UartProfile::CandidateReg0))))return fail();
    state_=response[6]==target?State::ReadAfter:State::WriteTemporary;break;
   }
   case State::WriteTemporary:{
    // C2: volatile UART register only; never touch RF settings or flash.
    const uint8_t cmd[]={0xC2,0x03,0x01,target};uint8_t response[4]{};
    if(!p.exchangeExact(cmd,sizeof(cmd),response,sizeof(response),StepLimit)||
       response[0]!=0xC1||response[1]!=0x03||response[2]!=0x01||response[3]!=target)return fail();
    state_=State::ReadAfter;break;
   }
   case State::ReadAfter:{
    uint8_t response[12]{};if(!readProfile(p,response)||!matches(response,target))return fail();
    state_=State::RuntimeBaud;break;
   }
   case State::RuntimeBaud:
    if(!p.baudVerified(targetBaud_,StepLimit))return fail();
    state_=State::NormalMode;break;
   case State::NormalMode:
    if(!p.modeVerified(false,StepLimit))return fail();
    state_=State::VerifyRx;break;
   case State::VerifyRx:
    // Fixture/peer receive evidence only; no outgoing test RF frame here.
    if(!p.receiveFixtureVerified(targetBaud_,StepLimit))return fail();
    state_=State::VerifiedLocked;break;
   default:return fail();
  }
  return p.lockedAndIsolated()?true:fail();
 }
private:
 template<class Port> static bool readProfile(Port& p,uint8_t (&out)[12]){
  const uint8_t cmd[]={0xC1,0x00,0x09};return p.exchangeExact(cmd,sizeof(cmd),out,sizeof(out),StepLimit);
 }
 static bool matches(const uint8_t (&r)[12],uint8_t reg0){
  const uint8_t expected[]={0xC1,0x00,0x09,0x00,0x00,0x00,reg0,0xC3,0x12,0x00,0x00,0x00};
  for(unsigned i=0;i<sizeof(expected);++i)if(r[i]!=expected[i])return false;
  return true;
 }
 bool fail(){state_=State::FaultLocked;return false;}
 State state_=State::Idle;uint32_t started_=0,targetBaud_=0;bool recovering_=false;const void* port_=nullptr;
};
}
#endif // FLYTEC_E22_UART_TRANSITION_SHARED_H
