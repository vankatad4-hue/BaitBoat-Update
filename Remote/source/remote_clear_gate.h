#pragma once
#include <stdint.h>
namespace flytec_rtos {
// Owner-only, after authenticated primary RX and ACK-tracker validation.
// No local TX completion can grant movement authority.
struct ClearAuthorization {
 bool session=false,primaryFresh=false,neutral=false,stationary=false,safe=false;
 bool valid()const{return session&&primaryFresh&&neutral&&stationary&&safe;}
};
class RemoteClearGate {
public:
 enum class State:uint8_t{Idle,WaitingAck,WaitingStatus,Ready,Failed};
 static constexpr uint32_t Deadline=1500U;
 void reset(){state_=State::Idle;id_=0;epoch_=0;started_=ackAt_=statusAt_=0;}
 State state()const{return state_;}
 bool pending()const{return state_==State::WaitingAck||state_==State::WaitingStatus||state_==State::Ready;}
 uint16_t id()const{return id_;}
 bool begin(uint16_t id,uint8_t epoch,uint32_t now,bool confirmedStopped,const ClearAuthorization& a){
  if(state_!=State::Idle||!id||!epoch||!confirmedStopped||!a.valid())return false;
  id_=id;epoch_=epoch;started_=now;state_=State::WaitingAck;return true;
 }
 // Only an EXECUTED ACK for CLEAR_STOP is accepted as clearance evidence.
 // ACCEPTED only means queued. Rejection/ambiguity leaves the latch closed.
 bool ack(uint16_t id,uint8_t epoch,bool primary,bool executed,bool rejected,uint32_t captured,uint32_t now){
  if(state_!=State::WaitingAck||id!=id_||epoch!=epoch_||!primary||!fresh(captured,now))return false;
  if(rejected){state_=State::Failed;return true;}
  if(!executed)return false;
  ackAt_=captured;state_=State::WaitingStatus;return true;
 }
 bool status(uint8_t epoch,bool primary,bool released,bool safe,uint32_t captured,uint32_t now){
  if(state_!=State::WaitingStatus||epoch!=epoch_||!primary||!fresh(captured,now)||!after(captured,ackAt_))return false;
  if(!released||!safe)return false;
  statusAt_=captured;state_=State::Ready;return true;
 }
 bool maintain(uint32_t now,const ClearAuthorization& a){
  if(!pending())return state_!=State::Failed;
  if(!a.valid()||uint32_t(now-started_)>=Deadline){state_=State::Failed;return false;}
  return true;
 }
 bool ready(uint32_t now,const ClearAuthorization& a)const{
  return state_==State::Ready&&a.valid()&&uint32_t(now-started_)<Deadline&&uint32_t(now-statusAt_)<100U;
 }
private:
 static bool after(uint32_t a,uint32_t b){const uint32_t d=a-b;return d&&d<0x80000000U;}
 bool fresh(uint32_t captured,uint32_t now)const{return uint32_t(now-captured)<100U&&uint32_t(captured-started_)<Deadline&&uint32_t(now-started_)<Deadline;}
 State state_=State::Idle;uint16_t id_=0;uint8_t epoch_=0;uint32_t started_=0,ackAt_=0,statusAt_=0;
};
}
