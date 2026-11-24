#pragma once
#include "r30h118_transport.h"
namespace flytec_r30h118 {
// Owner only. Admission to queue is not physical STOP confirmation.
template<class Frame> class StopRetry {
 Frame frame_{};
 uint32_t started_=0;
 bool pending_=false;
public:
 bool arm(Transport<Frame>& bus,uint8_t epoch,const Frame &frame,uint32_t now){
  pending_=false;
  bus.latchStop();
  if(!bus.invalidate(epoch))return false;
  frame_=frame;started_=now;pending_=true;return true;
 }
 bool service(Transport<Frame>& bus,uint32_t now,uint32_t maxAge){
  if(!pending_)return false;
  if(!maxAge||uint32_t(now-started_)>=maxAge){pending_=false;return false;}
  if(!bus.submit(frame_,started_,true))return false;
  pending_=false;return true;
 }
 void reset(){pending_=false;}
 bool pending()const{return pending_;}
};
}
