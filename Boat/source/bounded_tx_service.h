#pragma once
#include "radio_frame_handoff.h"
// Dormant: one nonblocking admission attempt per link per tick.
// Generation snapshot must be serialized with session transition before activation.
namespace flytec_rtos {
class BoundedTxService {
public:
 using Start=bool(*)(void*,const TxRequest&);
 bool tick(RadioFrameHandoff& h,PhyLink link,uint32_t generation,Start start,void* ctx){
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
  if(r.generation!=generation||r.link!=link){
   const TxCompletion c{r.ticket,r.generation,r.link,false};
   s.pending=false;return h.completions.offer(c);
  }
  if(start(ctx,r))s.pending=false;
  return true;
 }
private:
 struct Slot{TxRequest request{};bool pending=false;};
 struct Slots{Slot critical,ordinary;};
 Slots primary_,backup_;
};
}
