#pragma once
#include "radio_handshake.h"
#include "radio_command_codec.h"
#include "radio_payload_codec.h"
#include "mission_wire.h"
#include "mission_control_wire.h"
#include "mission_stick.h"
#include "mission_neutral_wire.h"
// Test-only Boat peer. No hardware, motor outputs, GPIO, provisioned keys.
namespace flytec_rtos {
class RemoteMockPeer {
public:
 explicit RemoteMockPeer(SessionHmac mac):mac_(mac){}
 const uint8_t* root()const{return root_;}
 bool exchange(const RadioFrame& f,RadioFrame& response,bool backup,uint32_t now){
  const uint8_t flags=backup?RADIO_FLAG_BACKUP_LINK:0;
  if(f.sourceId!=1||f.destinationId!=2||f.flags!=flags)return false;
  if(f.type==RADIO_MSG_SESSION_HELLO||f.type==RADIO_MSG_SESSION_PROOF){
   if(f.commandId||f.sequence||f.sessionEpoch||!radio_frameVerify(f,root_))return false;
   SessionHandshakePacket p{};bool fresh=false;
   if(!session_.boatReceive((SessionHandshakeMessage)f.type,f.payload,456,root_,mac_,now,true,p,fresh,SessionHandshake::BackupWindowMs))return false;
   if(fresh){session_.frameKey(key_);sequence_=0;rx_.resetAfterHandshake();mission_.reset();resetMission();}
   radio_frameInit(response,(RadioMessageType)p.type,2,1,0,0,now,0);
   memcpy(response.payload,p.payload,8);response.flags=flags;return radio_frameSeal(response,root_);
  }
  if(!session_.established()||f.sessionEpoch!=1||!radio_frameVerify(f,key_)||!rx_.accept(f.sequence))return false;
  if(f.type==RADIO_MSG_MISSION_PREPARE){
   const uint16_t id=uint16_t(f.payload[1])|(uint16_t(f.payload[2])<<8);
   if(backup||!id||id!=f.commandId||f.payload[0]<MISSION_BEGIN||f.payload[0]>MISSION_CANCEL)return false;
   const auto result=mission_.ingest(f.payload,now,true,stopped_);
   if(result==MISSION_PREPARED){missionId_=id;phase_=MC_PREPARED;stageAt_=now;lease_.reset();lastRequest_=0;}
   if(result==MISSION_CANCELLED){phase_=MC_ABORTED;lease_.reset();}
   radio_frameInit(response,RADIO_MSG_MISSION_ACK,2,1,++sequence_,id,now,1);
   return missionAck_encode(id,static_cast<MissionOpcode>(f.payload[0]),result,response.payload)&&radio_frameSeal(response,key_);
  }
  // Test-only controller model; real Boat controller acceptance still needs CI.
  if(f.type==RADIO_MSG_MISSION_STICKS){
   MissionStickInput s{};
   if(backup||stopped_||!missionId_||f.commandId!=missionId_||!missionStickDecode(f.payload,s)||
      !s.neutral||!lease_.receive(s,missionId_,now,true))return false;
   radio_frameInit(response,RADIO_MSG_MISSION_NEUTRAL_ACK,2,1,++sequence_,missionId_,now,1);
   return missionNeutralEncode(missionId_,f.sequence,response.payload)&&radio_frameSeal(response,key_);
  }
  if(f.type==RADIO_MSG_MISSION_CONTROL){
   MissionControlRequest q{};
   if(backup||!missionControlDecode(f.payload,q)||q.transaction!=missionId_||f.commandId!=missionId_||q.request<lastRequest_)return false;
   if(q.request==lastRequest_){if(q.op!=lastOp_)return false;}
   else{
    bool ok=false;
    if(q.op==MCTRL_CANCEL){phase_=MC_ABORTED;lease_.reset();ok=true;}
    else if(!stopped_){
     if(q.op==MCTRL_START&&phase_==MC_PREPARED&&uint32_t(now-stageAt_)<10000U&&lease_.neutral(missionId_,now)){phase_=MC_OUTBOUND;ok=true;}
     else if(q.op==MCTRL_PAUSE&&phase_==MC_OUTBOUND){phase_=MC_PAUSED;lease_.reset();ok=true;}
     else if(q.op==MCTRL_RESUME&&phase_==MC_PAUSED&&lease_.neutral(missionId_,now)){phase_=MC_OUTBOUND;ok=true;}
    }
    lastRequest_=q.request;lastOp_=q.op;lastResult_=ok?MC_ACCEPTED:MC_REJECTED;
   }
   radio_frameInit(response,RADIO_MSG_MISSION_FEEDBACK,2,1,++sequence_,missionId_,now,1);
   return missionFeedbackEncode({missionId_,phase_,uint8_t(q.op),q.request,lastResult_},response.payload)&&radio_frameSeal(response,key_);
  }
  if(f.type==RADIO_MSG_ACTION&&f.payload[0]==CMD_EMERGENCY_STOP){
   BoatCommand c{};if(!radio_decodeCommand(f,1,2,now,0,backup,c,key_))return false;
   stopped_=true;mission_.reset();resetMission();lastStop_=uint16_t(c.commandId);
   radio_frameInit(response,RADIO_MSG_ACK,2,1,++sequence_,lastStop_,now,1);
   response.flags=flags;response.payload[1]=RADIO_ACK_ACCEPTED;
   response.payload[2]=CMD_EMERGENCY_STOP;response.payload[3]=uint8_t(lastStop_);
   response.payload[4]=uint8_t(lastStop_>>8);response.payload[5]=1;
   return radio_frameSeal(response,key_);
  }
  if(f.type==RADIO_MSG_ACTION||f.type==RADIO_MSG_CONTROL){
   BoatCommand c{};if(backup||!radio_decodeCommand(f,1,2,now,0,false,c,key_))return false;
   if(c.type==CMD_MANUAL_MOTORS){if(stopped_||missionId_)return false;return status(response,false,now);}
   bool executed=false;
   if(c.type==CMD_CLEAR_STOP&&stopped_){stopped_=false;executed=true;}
   else if(c.type>=CMD_RELEASE_HOPPER_LEFT&&c.type<=CMD_RELEASE_HOOK_RIGHT&&!stopped_&&!missionId_){++mockReleases_;executed=true;}
   // The real Boat currently rejects SET_MODE(MANUAL); do not simulate success.
   radio_frameInit(response,RADIO_MSG_ACK,2,1,++sequence_,uint16_t(c.commandId),now,1);
   response.payload[1]=executed?RADIO_ACK_EXECUTED:RADIO_ACK_REJECTED_SAFETY;
   response.payload[2]=uint8_t(c.type);response.payload[3]=uint8_t(c.commandId);
   response.payload[4]=uint8_t(c.commandId>>8);response.payload[5]=2;
   return radio_frameSeal(response,key_);
  }
  if(f.type!=RADIO_MSG_HEARTBEAT)return false;
  RadioDecodedPayload decoded{};if(!radio_decodeHeartbeatPayload(f.payload,decoded))return false;
  return status(response,backup,now);
 }
 bool status(RadioFrame& f,bool backup,uint32_t now){
  if(!session_.established())return false;
  radio_frameInit(f,RADIO_MSG_TELEMETRY,2,1,++sequence_,0,now,1);
  f.flags=backup?RADIO_FLAG_BACKUP_LINK:0;f.payload[0]=1;
  f.payload[1]=MODE_MANUAL;f.payload[2]=stopped_?1:0;f.payload[3]=stopped_?0:1;f.payload[4]=80;
  return radio_frameSeal(f,key_);
 }
 uint16_t lastStop()const{return lastStop_;}
 unsigned mockReleases()const{return mockReleases_;}
 bool missionStatus(RadioFrame& f,uint32_t now){
  if(!missionId_)return false;
  if(phase_==MC_PREPARED&&uint32_t(now-stageAt_)>=10000U)phase_=MC_ABORTED;
  radio_frameInit(f,RADIO_MSG_MISSION_FEEDBACK,2,1,++sequence_,missionId_,now,1);
  return missionFeedbackEncode({missionId_,phase_,0,0,MC_STATUS},f.payload)&&radio_frameSeal(f,key_);
 }
 bool prepared(MissionPlan& out,uint32_t now)const{return mission_.getPrepared(out,now);}
private:
 void resetMission(){missionId_=lastRequest_=0;phase_=MC_IDLE;lease_.reset();}
 MissionStickLease lease_;uint16_t missionId_=0,lastRequest_=0;uint32_t stageAt_=0;
 MissionControlPhase phase_=MC_IDLE;MissionControlOp lastOp_=MCTRL_CANCEL;MissionControlResult lastResult_=MC_STATUS;
 SessionHmac mac_;SessionHandshake session_;RadioSessionCounter rx_;MissionAssembler mission_;
 uint8_t root_[32]{42},key_[32]{};uint16_t sequence_=0,lastStop_=0;bool stopped_=false;unsigned mockReleases_=0;
};
}
