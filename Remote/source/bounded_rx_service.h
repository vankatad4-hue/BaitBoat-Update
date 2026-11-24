#pragma once
#include "radio_frame_handoff.h"
// Dormant PHY adapter: callback must be bounded and return false when empty.
// At most two frames per link per invocation; raw RX never authorizes STOP.
namespace flytec_rtos {
class BoundedRxService {
public:
 using Receive=bool(*)(void*,RadioFrame&);
 bool tick(RadioFrameHandoff& h,PhyLink link,uint32_t now,Receive receive,void* ctx){
  if(h.fault()||!receive)return false;
  if(link!=PhyLink::Primary&&link!=PhyLink::Backup)return false;
  auto& q=link==PhyLink::Primary?h.primaryRx:h.backupRx;
  for(unsigned i=0;i<2;++i){
   RxFrame r{};r.link=link;r.receivedMs=now;
   if(!receive(ctx,r.frame))break;
   if(!q.offer(r))return false;
  }
  return true;
 }
};
}
