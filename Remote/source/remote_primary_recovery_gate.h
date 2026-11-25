#pragma once
#include <stdint.h>
namespace flytec_rtos {
// Experimental evidence gate only. Owner supplies verified primary challenge
// echoes, never generic STATUS or a local nRF hardware ACK. No movement authority.
class PrimaryRecoveryGate {
public:
 static constexpr uint32_t ReplyAge=250U,MinSpan=1000U,MaxGap=750U;
 void reset(){*this=PrimaryRecoveryGate{};}
 bool begin(uint8_t epoch,uint32_t generation){
  reset();if(!epoch||!generation)return false;epoch_=epoch;generation_=generation;return true;
 }
 bool offer(uint16_t sequence,uint32_t now,uint8_t epoch,uint32_t generation){
  if(!matches(epoch,generation)||!sequence||sequence<=high_)return false;
  expire(now);if(pending_)return false;high_=sequence;offeredAt_=now;pending_=true;return true;
 }
 bool observe(uint16_t echoed,uint32_t captured,uint32_t now,uint8_t epoch,uint32_t generation,bool authenticatedPrimary){
  if(!authenticatedPrimary||!matches(epoch,generation)||!pending_||echoed!=high_||
     uint32_t(now-captured)>=100U||uint32_t(captured-offeredAt_)>=ReplyAge||uint32_t(now-offeredAt_)>=ReplyAge)return false;
  pending_=false;expire(now);
  if(!count_)first_=now;
  if(count_<255)++count_;
  last_=now;return true;
 }
 void expire(uint32_t now){
  if(pending_&&uint32_t(now-offeredAt_)>=ReplyAge){pending_=false;count_=0;}
  if(count_&&uint32_t(now-last_)>=MaxGap)count_=0;
 }
 bool ready(uint32_t now,uint8_t epoch,uint32_t generation)const{
  return matches(epoch,generation)&&count_>=3&&uint32_t(last_-first_)>=MinSpan&&uint32_t(now-last_)<MaxGap;
 }
private:
 bool matches(uint8_t e,uint32_t g)const{return epoch_&&e==epoch_&&g==generation_;}
 uint8_t epoch_=0,count_=0;uint16_t high_=0;uint32_t generation_=0,offeredAt_=0,first_=0,last_=0;bool pending_=false;
};
}
