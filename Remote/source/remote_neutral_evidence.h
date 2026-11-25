#pragma once
#include <stdint.h>
namespace flytec_rtos {
// Core 1 only. Receipt is evidence of past acceptance, not a promise of
// future delivery or remaining Boat lease. Boat rechecks its actual lease.
class RemoteNeutralEvidence {
public:
 static constexpr uint32_t Age=100U;
 void reset(){*this=RemoteNeutralEvidence{};}
 bool offered(uint16_t transaction,uint16_t sequence,uint8_t epoch,uint32_t generation,uint32_t now){
  if(!transaction||!sequence||!epoch||!generation)return false;
  transaction_=transaction;sequence_=sequence;epoch_=epoch;generation_=generation;at_=now;seen_=false;return true;
 }
 bool receipt(uint16_t transaction,uint16_t sequence,uint8_t epoch,uint32_t generation,uint32_t captured,uint32_t now,bool authenticatedPrimary){
  if(!authenticatedPrimary||seen_||!sequence_||transaction!=transaction_||sequence!=sequence_||epoch!=epoch_||generation!=generation_||
     uint32_t(captured-at_)>=Age||uint32_t(now-at_)>=Age||uint32_t(now-captured)>=Age)return false;
  seen_=true;return true;
 }
 bool fresh(uint16_t transaction,uint8_t epoch,uint32_t generation,uint32_t now,bool localNeutral)const{
  return localNeutral&&seen_&&transaction==transaction_&&epoch==epoch_&&generation==generation_&&uint32_t(now-at_)<Age;
 }
private:
 uint16_t transaction_=0,sequence_=0;uint8_t epoch_=0;uint32_t generation_=0,at_=0;bool seen_=false;
};
}
