#pragma once
#include "mission_protocol.h"
// Portable sender: no radio or actuation. The transport must authenticate
// peer/session/replay and convey an exact transaction + opcode response.
enum MissionUploadState:uint8_t{UPLOAD_IDLE,UPLOAD_ACTIVE,UPLOAD_PREPARED,UPLOAD_FAILED};
class MissionUpload{
public:
 bool begin(const MissionPlan &p,uint32_t now,bool safe){
  if(!safe||!missionPlan_valid(p)||state_==UPLOAD_ACTIVE||state_==UPLOAD_PREPARED)return false;
  plan_=p;op_=MISSION_BEGIN;start_=now;last_=now;attempts_=0;waiting_=false;state_=UPLOAD_ACTIVE;return true;
 }
 bool next(uint32_t now,bool safe,uint8_t out[8]){
  if(state_!=UPLOAD_ACTIVE||!out)return false;
  if(!safe||now-start_>=9000U){fail();return false;}
  if(waiting_&&now-last_<500U)return false;
  if(attempts_>=3){fail();return false;}
  if(!missionPlan_encode(plan_,op_,out)){fail();return false;}
  ++attempts_;waiting_=true;last_=now;return true;
 }
 bool acknowledge(uint16_t id,MissionOpcode op,MissionReceive result,uint32_t now,bool authenticated){
  if(state_!=UPLOAD_ACTIVE||!waiting_||!authenticated||id!=plan_.transaction||op!=op_||now-start_>=9000U)return false;
  if(result==MISSION_REJECTED||result==MISSION_CANCELLED){fail();return true;}
  if(op==MISSION_COMMIT){
   // An exact duplicate COMMIT response means the receiver already prepared it.
   if(result!=MISSION_PREPARED&&result!=MISSION_DUPLICATE)return false;
   state_=UPLOAD_PREPARED;waiting_=false;return true;
  }
  if(result!=MISSION_PARTIAL&&result!=MISSION_DUPLICATE)return false;
  op_=static_cast<MissionOpcode>(static_cast<uint8_t>(op_)+1U);waiting_=false;attempts_=0;return true;
 }
 void fail(){state_=UPLOAD_FAILED;waiting_=false;}
 MissionUploadState state()const{return state_;}
private:
 MissionPlan plan_{};MissionOpcode op_=MISSION_BEGIN;uint32_t start_=0,last_=0;
 uint8_t attempts_=0;bool waiting_=false;MissionUploadState state_=UPLOAD_IDLE;
};
