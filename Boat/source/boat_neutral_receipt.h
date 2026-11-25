#pragma once
#include "mission_stick.h"
#include "mission_neutral_wire.h"
#include "radio_protocol.h"
// One RX-pass only, best effort. No queue or retry, no movement authority.
// Call offer only after HMAC, replay and actual lease acceptance.
class BoatNeutralReceipt {
public:
 static constexpr uint32_t MaxAge=50U;
 void reset(){pending_=false;}
 bool offer(const MissionStickInput& s,uint16_t sequence,uint8_t epoch,uint32_t generation,uint32_t captured,uint32_t now,bool acceptedPrimary){
  reset();
  if(!acceptedPrimary||!s.transaction||!s.neutral||s.left||s.right||!sequence||!epoch||uint32_t(now-captured)>=MaxAge)return false;
  id_=s.transaction;sequence_=sequence;epoch_=epoch;generation_=generation;captured_=captured;pending_=true;return true;
 }
 // Consume even when busy; never delay STOP/STATUS/handshake for a receipt.
 bool take(uint16_t transaction,uint8_t epoch,uint32_t generation,uint32_t now,bool ready,bool phaseAllowed,const MissionStickLease& lease,uint8_t out[8]){
  if(!pending_)return false;
  reset();
  if(!ready||!phaseAllowed||transaction!=id_||epoch!=epoch_||generation!=generation_||uint32_t(now-captured_)>=MaxAge||!lease.neutral(transaction,now))return false;
  return missionNeutralEncode(id_,sequence_,out);
 }
private:
 bool pending_=false;uint16_t id_=0,sequence_=0;uint8_t epoch_=0;uint32_t generation_=0,captured_=0;
};
