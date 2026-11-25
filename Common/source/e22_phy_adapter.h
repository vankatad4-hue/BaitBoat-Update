#pragma once
#include "bounded_tx_service.h"
#include "bounded_rx_service.h"
#include "e22_tx_state.h"
#include "lora_link_policy.h"
#include "mission_control_wire.h"
// Dormant: single PHY owner. LocalReady is NOT RF delivery or peer ACK.
// Driver initialization, bus ownership and RF signoff remain external.
namespace flytec_rtos {
template<class Driver> class E22PhyAdapter {
public:
 // Optional minimum spacing at the PHY boundary (Remote worker: 3000 ms).
 explicit E22PhyAdapter(Driver& driver,uint32_t minimumGapMs=0):driver_(driver),minimumGapMs_(minimumGapMs){}
 using Adaptive=E22AdaptiveAdmission<Driver>;
 using Traffic=LoRaAirtimePolicy::Traffic;
 // Only the trusted application/worker boundary supplies classification of
 // immutable, owner-sealed requests. No classification of raw RX frames.
 using Classify=bool(*)(void*,const TxRequest&,Traffic&);
 // Remote application metadata is emitted only after owner admission. A
 // raw packet, stale generation or unknown classification cannot use reserve.
 static bool classifyRemoteOwner(void*,const TxRequest& r,Traffic& out){
  const auto& f=r.frame;
  if(r.link!=PhyLink::Backup||!r.ticket||!r.generation||f.sourceId!=1||f.destinationId!=2||
     !radioLinkFlagMatches(f.flags,true))return false;
  switch(r.traffic){
   case TxTraffic::Routine:
    if(f.commandId)return false;
    if(f.type==RADIO_MSG_HEARTBEAT){if(!f.sessionEpoch||!f.sequence)return false;}
    else if((f.type!=RADIO_MSG_SESSION_HELLO&&f.type!=RADIO_MSG_SESSION_PROOF)||f.sessionEpoch||f.sequence)return false;
    out=Traffic::Routine;return true;
   case TxTraffic::BackupMission:{
    MissionControlRequest q{};
    if(f.type!=RADIO_MSG_MISSION_CONTROL||!f.commandId||!f.sessionEpoch||!f.sequence||
       !missionControlDecode(f.payload,q)||q.transaction!=f.commandId||(q.op!=MCTRL_PAUSE&&q.op!=MCTRL_CANCEL))return false;
    out=Traffic::Mission;return true;
   }
   case TxTraffic::EmergencyStop:
    if(f.type!=RADIO_MSG_ACTION||!f.commandId||!f.sessionEpoch||!f.sequence||f.payload[0]!=RADIO_ACTION_STOP||f.payload[6]||f.payload[7])return false;
    out=Traffic::Emergency;return true;
   default:return false;
  }
 }
 E22PhyAdapter(Driver& driver,Adaptive& admission):E22PhyAdapter(driver,admission,&classifyRemoteOwner,nullptr){}
 E22PhyAdapter(Driver& driver,Adaptive& admission,Classify classify,void* context):
  driver_(driver),minimumGapMs_(0),adaptive_(&admission),classify_(classify),classifyContext_(context){
  adaptiveFault_=!admission.belongsTo(driver)||!classify;
  adaptiveStart_=&callAdaptiveStart;adaptivePoll_=&callAdaptivePoll;
 }
 E22PhyAdapter(const E22PhyAdapter&)=delete;
 E22PhyAdapter& operator=(const E22PhyAdapter&)=delete;
 bool tick(RadioFrameHandoff& h,TxGenerationGate& gate,uint32_t now){
  constexpr auto link=PhyLink::Backup;
  if(adaptiveFault_||(h.fault(link)&&!outstanding_))return false;
  driver_.update();
  if(outstanding_){
   bool terminal=false,local=false;
   if(adaptive_){
    const auto outcome=adaptivePoll_(*adaptive_,now);
    terminal=outcome!=Adaptive::Poll::Pending;
    local=outcome==Adaptive::Poll::LocalComplete;
    if(terminal&&!local)adaptiveFault_=true;
   }else{
    terminal=!driver_.txPending()||driver_.transportFault();
    local=driver_.configured()&&!driver_.transportFault()&&driver_.txOutcome()==E22TxState::Outcome::LocalReady;
   }
   if(terminal){
    outstanding_=false;lastPossibleTx_=now;hasPossibleTx_=true;
    if(!h.offerCompletion(link,TxCompletion{request_.ticket,request_.generation,link,false,
          local?TxEvidence::LocalReady:TxEvidence::None}))return false;
   }
  }
  if(adaptiveFault_||!driver_.configured()||driver_.transportFault()||h.fault(link))return false;
  if(!rx_.tick(h,link,now,&receive,this))return false;
  if(outstanding_||driver_.txPending())return true;
  Context ctx{this,&h,now};
  const bool ok=tx_.tick(h,link,gate,&start,&ctx,now);
  return ok&&!adaptiveFault_&&!h.fault(link)&&!driver_.transportFault();
 }
private:
 struct Context{E22PhyAdapter* self;RadioFrameHandoff* handoff;uint32_t now;};
 static bool receive(void* p,RadioFrame& f){return static_cast<E22PhyAdapter*>(p)->driver_.receiveFrame(f);}
 static bool start(void* p,const TxRequest& r){
  auto& c=*static_cast<Context*>(p);auto& self=*c.self;
  using A=typename Driver::Admission;
  if(self.hasPossibleTx_&&uint32_t(c.now-self.lastPossibleTx_)<self.minimumGapMs_)return false;
  if(self.adaptive_){
   Traffic traffic=Traffic::Routine;
   using R=typename Adaptive::Result;
   const auto result=self.classify_(self.classifyContext_,r,traffic)?
     self.adaptiveStart_(*self.adaptive_,r.frame,traffic,c.now):R::Denied;
   if(result==R::Busy||result==R::DutyLimited)return false;
   if(result==R::Accepted){self.request_=r;self.outstanding_=true;return true;}
   if(result==R::Fault)self.adaptiveFault_=true;
   c.handoff->offerCompletion(PhyLink::Backup,TxCompletion{r.ticket,r.generation,PhyLink::Backup,false});
   return true;
  }
  const auto result=self.driver_.trySendFrame(r.frame,c.now);
  if(result==A::Accepted||result==A::Uncertain){self.lastPossibleTx_=c.now;self.hasPossibleTx_=true;}
  if(result==A::Accepted){self.request_=r;self.outstanding_=true;return true;}
  if(result==A::Busy||result==A::DutyLimited)return false;
  // Terminal rejection consumes the request and emits a failure, never retries
  // an ambiguous write. Sticky overflow is reported by tick.
  c.handoff->offerCompletion(PhyLink::Backup,TxCompletion{r.ticket,r.generation,PhyLink::Backup,false});
  return true;
 }
 // Function pointers instantiate the additional driver requirements only
 // when the opt-in constructor is used; legacy driver mocks remain valid.
 static typename Adaptive::Result callAdaptiveStart(Adaptive& a,const RadioFrame& f,Traffic t,uint32_t now){return a.start(f,t,now);}
 static typename Adaptive::Poll callAdaptivePoll(Adaptive& a,uint32_t now){return a.poll(now);}
 Driver& driver_;
 BoundedTxService tx_;
 BoundedRxService rx_;
 TxRequest request_{};
 const uint32_t minimumGapMs_;
 uint32_t lastPossibleTx_=0;
 bool outstanding_=false,hasPossibleTx_=false;
 Adaptive* adaptive_=nullptr;Classify classify_=nullptr;void* classifyContext_=nullptr;
 typename Adaptive::Result (*adaptiveStart_)(Adaptive&,const RadioFrame&,Traffic,uint32_t)=nullptr;
 typename Adaptive::Poll (*adaptivePoll_)(Adaptive&,uint32_t)=nullptr;
 bool adaptiveFault_=false;
};
}
