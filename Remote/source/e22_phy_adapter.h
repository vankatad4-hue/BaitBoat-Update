#pragma once
#include "bounded_tx_service.h"
#include "bounded_rx_service.h"
#include "e22_tx_state.h"
// Dormant: single PHY owner. LocalReady is NOT RF delivery or peer ACK.
// Driver initialization, bus ownership and RF signoff remain external.
namespace flytec_rtos {
template<class Driver> class E22PhyAdapter {
public:
 explicit E22PhyAdapter(Driver& driver):driver_(driver){}
 E22PhyAdapter(const E22PhyAdapter&)=delete;
 E22PhyAdapter& operator=(const E22PhyAdapter&)=delete;
 bool tick(RadioFrameHandoff& h,TxGenerationGate& gate,uint32_t now){
  constexpr auto link=PhyLink::Backup;
  if(h.fault(link)&&!outstanding_)return false;
  driver_.update();
  if(outstanding_&&(!driver_.txPending()||driver_.transportFault())){
   const auto local=driver_.configured()&&!driver_.transportFault()&&
     driver_.txOutcome()==E22TxState::Outcome::LocalReady;
   outstanding_=false;
   if(!h.offerCompletion(link,TxCompletion{request_.ticket,request_.generation,link,false,
          local?TxEvidence::LocalReady:TxEvidence::None}))return false;
  }
  if(!driver_.configured()||driver_.transportFault()||h.fault(link))return false;
  if(!rx_.tick(h,link,now,&receive,this))return false;
  if(outstanding_||driver_.txPending())return true;
  Context ctx{this,&h,now};
  const bool ok=tx_.tick(h,link,gate,&start,&ctx,now);
  return ok&&!h.fault(link)&&!driver_.transportFault();
 }
private:
 struct Context{E22PhyAdapter* self;RadioFrameHandoff* handoff;uint32_t now;};
 static bool receive(void* p,RadioFrame& f){return static_cast<E22PhyAdapter*>(p)->driver_.receiveFrame(f);}
 static bool start(void* p,const TxRequest& r){
  auto& c=*static_cast<Context*>(p);auto& self=*c.self;
  using A=typename Driver::Admission;
  const auto result=self.driver_.trySendFrame(r.frame,c.now);
  if(result==A::Accepted){self.request_=r;self.outstanding_=true;return true;}
  if(result==A::Busy||result==A::DutyLimited)return false;
  // Terminal rejection consumes the request and emits a failure, never retries
  // an ambiguous write. Sticky overflow is reported by tick.
  c.handoff->offerCompletion(PhyLink::Backup,TxCompletion{r.ticket,r.generation,PhyLink::Backup,false});
  return true;
 }
 Driver& driver_;
 BoundedTxService tx_;
 BoundedRxService rx_;
 TxRequest request_{};
 bool outstanding_=false;
};
}
