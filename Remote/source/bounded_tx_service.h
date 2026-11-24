#pragma once
#include "radio_frame_handoff.h"
// Dormant bounded service. Driver adapter must implement nonblocking tryStart:
// true = accepted, false = busy (no side effects). Completion of accepted
// frames is reported separately by the driver, not synthesized here.
// Generation is a snapshot; production must serialize session transition
// with admission before enabling physical TX.
namespace flytec_rtos {
class BoundedTxService {
public:
 using Start=bool(*)(void*,const TxRequest&);
 bool tick(RadioFrameHandoff& h,PhyLink link,uint32_t generation,Start start,void* ctx){
  if(h.fault()||!start)return false;
  Slot* s=link==PhyLink::Primary?&primary_:link==PhyLink::Backup?&backup_:nullptr;
  if(!s)return false;
  if(!s->pending){
   if(!h.takeTx(link,s->request))return true;
   s->pending=true;
  }
  const auto& r=s->request;
  if(r.generation!=generation||r.link!=link){
   const TxCompletion c{r.ticket,r.generation,r.link,false};
   s->pending=false;
   return h.completions.offer(c);
  }
  if(start(ctx,r))s->pending=false;
  return true;
 }
private:
 struct Slot{TxRequest request{};bool pending=false;};
 Slot primary_,backup_;
};
}
