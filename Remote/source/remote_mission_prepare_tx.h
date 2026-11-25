#pragma once
#include "remote_async_mission_upload.h"
#include "radio_protocol.h"
#include <string.h>
namespace flytec_rtos {
// Shape only: authentication and stopped/stationary authority belong to the owner.
inline bool scopedPrepareFrame(const RadioFrame& f){
 if(f.type!=RADIO_MSG_MISSION_PREPARE||f.sourceId!=1||f.destinationId!=2||f.flags||
    !f.sequence||!f.sessionEpoch||!f.commandId||f.payload[0]<MISSION_BEGIN||f.payload[0]>MISSION_COMMIT||
    f.commandId!=(uint16_t(f.payload[1])|(uint16_t(f.payload[2])<<8)))return false;
 if(f.payload[0]==MISSION_BEGIN){
  const uint16_t radius=uint16_t(f.payload[6])|(uint16_t(f.payload[7])<<8);
  return f.payload[3]<8&&f.payload[4]<16&&!(f.payload[5]&0xF0U)&&radius>=300&&radius<=1000;
 }
 return f.payload[3]==0;
}
template<class Tx> class RemoteMissionPrepareTx {
public:
 explicit RemoteMissionPrepareTx(Tx& tx):tx_(tx){}
 AsyncMissionUpload& upload(){return upload_;}
 const AsyncMissionUpload& upload()const{return upload_;}
 // False requires the owner to revoke the transport generation. No fallback.
 bool service(uint32_t now,bool safe,bool gap,uint8_t epoch,uint16_t& seq,uint16_t rekeyAt,const uint8_t key[32]){
  using S=AsyncMissionUpload::State;using L=typename Tx::Link;using Q=typename Tx::Submit;
  if(upload_.state()==S::Idle)return true;
  if(!upload_.maintain(now,safe))return false;
  if(!gap||tx_.pending(L::Primary))return true;
  uint8_t payload[8]{};
  if(!upload_.due(now,safe,payload))return upload_.state()!=S::Failed;
  if(!epoch||!key||rekeyAt<2||seq>=uint16_t(rekeyAt-1)||tx_.linkFault(L::Primary))return false;
  RadioFrame f{};radio_frameInit(f,RADIO_MSG_MISSION_PREPARE,1,2,uint16_t(seq+1),upload_.transaction(),now,epoch);
  memcpy(f.payload,payload,8);
  if(!scopedPrepareFrame(f)||!radio_frameSeal(f,key))return false;
  ++seq;uint32_t ticket=0;
  const auto result=tx_.submitPrepare(f,now,AsyncMissionUpload::TxLifetime,now,ticket,safe);
  if(result==Q::Busy)return true;
  if(result!=Q::Queued)return false;
  return upload_.queued(ticket,tx_.generation(),seq,epoch,now);
 }
 bool observeCompletion(const typename Tx::Result& c){
  using L=typename Tx::Link;using K=typename Tx::Kind;
  return c.link==L::Primary&&c.kind==K::Mission&&!c.stop&&c.messageType==RADIO_MSG_MISSION_PREPARE&&
   upload_.complete(c.ticket,c.generation,c.sequence,c.epoch,c.commandId,c.createdMs,c.lifetimeMs);
 }
private:
 Tx& tx_;AsyncMissionUpload upload_;
};
}
