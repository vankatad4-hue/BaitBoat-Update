#pragma once
#include "radio_frame_handoff.h"
#include "tx_generation_gate.h"
// Dormant: one nonblocking admission attempt per link per tick.
// Critical queue must contain only application-approved STOP-safe traffic.
namespace flytec_rtos {
class BoundedTxService {
public:
 using Start=bool(*)(void*,const TxRequest&);
 bool tick(RadioFrameHandoff& h,PhyLink link,TxGenerationGate& gate,Start start,void* ctx){
  if(h.fault()||!start)return false;
  if(link!=PhyLink::Primary&&link!=PhyLink::Backup)return false;
  auto& slots=link==PhyLink::Primary?primary_:backup_;
  auto& critical=link==PhyLink::Primary?h.primaryCritical:h.backupCritical;
  auto& ordinary=link==PhyLink::Primary?h.primaryTx:h.backupTx;
  if(!slots.critical.pending)slots.critical.pending=critical.take(slots.critical.request);
  if(!slots.critical.pending&&!slots.ordinary.pending)
   slots.ordinary.pending=ordinary.take(slots.ordinary.request);
  auto& s=slots.critical.pending?slots.critical:slots.ordinary;
  if(!s.pending)return true;
  const auto& r=s.request;
  Admission a{start,ctx,&r};
  const auto result=r.link==link?gate.tryAdmit(r.generation,&invoke,&a,slots.critical.pending):TxGenerationGate::Result::Stale;
  if(result==TxGenerationGate::Result::Stale||result==TxGenerationGate::Result::Inhibited){
   const TxCompletion c{r.ticket,r.generation,r.link,false};
   s.pending=false;return h.completions.offer(c);
  }
  if(result==TxGenerationGate::Result::Accepted)s.pending=false;
  return true;
 }
private:
 struct Admission{Start start;void* ctx;const TxRequest* request;};
 static bool invoke(void* p){auto& a=*static_cast<Admission*>(p);return a.start(a.ctx,*a.request);}
 struct Slot{TxRequest request{};bool pending=false;};
 struct Slots{Slot critical,ordinary;};
 Slots primary_,backup_;
};
}
