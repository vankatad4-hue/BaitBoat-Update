#pragma once
// Primary recovery evidence and experimental challenge/echo wire contract.
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
 bool awaiting(uint32_t now)const{return pending_&&uint32_t(now-offeredAt_)<ReplyAge;}
 bool matchesEcho(uint16_t echoed,uint32_t captured,uint32_t now,uint8_t epoch,uint32_t generation)const{
  return matches(epoch,generation)&&awaiting(now)&&echoed==high_&&
   uint32_t(now-captured)<100U&&uint32_t(captured-offeredAt_)<ReplyAge;
 }
 bool observe(uint16_t echoed,uint32_t captured,uint32_t now,uint8_t epoch,uint32_t generation,bool authenticatedPrimary){
  if(!authenticatedPrimary||!matchesEcho(echoed,captured,now,epoch,generation))return false;
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

#include <stdint.h>
namespace flytec_rtos {
// Experimental mock-only LINK_PROBE payload, never an action or STATUS.
// Distinct tag/version rejects any legacy or generic probe shape.
enum class RecoveryProbeKind:uint8_t{Challenge=1,Echo=2};
inline bool recoveryProbeEncode(RecoveryProbeKind kind,uint16_t sequence,uint8_t out[8]){
 if(!out||!sequence||(kind!=RecoveryProbeKind::Challenge&&kind!=RecoveryProbeKind::Echo))return false;
 out[0]=0x52;out[1]=0x43;out[2]=1;out[3]=uint8_t(kind);
 out[4]=uint8_t(sequence);out[5]=uint8_t(sequence>>8);out[6]=out[7]=0;
 return true;
}
inline bool recoveryProbeDecode(const uint8_t in[8],RecoveryProbeKind kind,uint16_t& sequence){
 sequence=0;
 if(!in||(kind!=RecoveryProbeKind::Challenge&&kind!=RecoveryProbeKind::Echo)||
    in[0]!=0x52||in[1]!=0x43||in[2]!=1||in[3]!=uint8_t(kind)||in[6]||in[7])return false;
 sequence=uint16_t(in[4])|(uint16_t(in[5])<<8);return sequence!=0;
}
}
