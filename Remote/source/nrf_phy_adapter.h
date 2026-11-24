#pragma once
#include "bounded_tx_service.h"
#include "bounded_rx_service.h"
// Dormant single PHY owner adapter for both nRF driver implementations.
// Caller must enforce existing RF/board gates and Remote I2C ownership.
// No driver initialization or task creation here.
namespace flytec_rtos {
template<class Driver> class NrfPhyAdapter {
public:
 explicit NrfPhyAdapter(Driver& driver):driver_(driver){}
 NrfPhyAdapter(const NrfPhyAdapter&)=delete;
 NrfPhyAdapter& operator=(const NrfPhyAdapter&)=delete;
 bool tick(RadioFrameHandoff& h,TxGenerationGate& gate,uint32_t now){
  constexpr auto link=PhyLink::Primary;
  if(h.fault(link)&&!outstanding_)return false;
  driver_.update();
  if(outstanding_&&(!driver_.ready()||!driver_.txPending())){
   const bool ok=driver_.ready()&&driver_.lastTxSuccess();
   const TxCompletion c{request_.ticket,request_.generation,link,ok,
                       ok?TxEvidence::NrfAck:TxEvidence::None};
   outstanding_=false;
   if(!h.offerCompletion(link,c))return false;
  }
  if(!driver_.ready()||h.fault(link))return false;
  if(!rx_.tick(h,link,now,&receive,this))return false;
  if(outstanding_||driver_.txPending())return true;
  return tx_.tick(h,link,gate,&start,this,now);
 }
private:
 static bool receive(void* p,RadioFrame& f){return static_cast<NrfPhyAdapter*>(p)->driver_.receiveFrame(f);}
 static bool start(void* p,const TxRequest& r){
  auto& self=*static_cast<NrfPhyAdapter*>(p);
  if(self.outstanding_||!self.driver_.ready()||self.driver_.txPending())return false;
  if(!self.driver_.sendFrame(r.frame))return false;
  self.request_=r;self.outstanding_=true;return true;
 }
 Driver& driver_;
 BoundedTxService tx_;
 BoundedRxService rx_;
 TxRequest request_{};
 bool outstanding_=false;
};
}
