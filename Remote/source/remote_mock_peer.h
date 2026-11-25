#pragma once
#include "radio_handshake.h"
#include "radio_command_codec.h"
#include "radio_payload_codec.h"
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
   if(fresh){session_.frameKey(key_);sequence_=0;rx_.resetAfterHandshake();}
   radio_frameInit(response,(RadioMessageType)p.type,2,1,0,0,now,0);
   memcpy(response.payload,p.payload,8);response.flags=flags;return radio_frameSeal(response,root_);
  }
  if(!session_.established()||f.sessionEpoch!=1||!radio_frameVerify(f,key_)||!rx_.accept(f.sequence))return false;
  if(f.type==RADIO_MSG_ACTION&&f.payload[0]==CMD_EMERGENCY_STOP){
   BoatCommand c{};if(!radio_decodeCommand(f,1,2,now,0,backup,c,key_))return false;
   stopped_=true;lastStop_=uint16_t(c.commandId);
   radio_frameInit(response,RADIO_MSG_ACK,2,1,++sequence_,lastStop_,now,1);
   response.flags=flags;response.payload[1]=RADIO_ACK_ACCEPTED;
   response.payload[2]=CMD_EMERGENCY_STOP;response.payload[3]=uint8_t(lastStop_);
   response.payload[4]=uint8_t(lastStop_>>8);response.payload[5]=1;
   return radio_frameSeal(response,key_);
  }
  if(f.type==RADIO_MSG_ACTION||f.type==RADIO_MSG_CONTROL){
   BoatCommand c{};if(backup||!radio_decodeCommand(f,1,2,now,0,false,c,key_))return false;
   if(c.type==CMD_MANUAL_MOTORS){if(stopped_)return false;return status(response,false,now);}
   bool executed=false;
   if(c.type==CMD_CLEAR_STOP&&stopped_){stopped_=false;executed=true;}
   else if(c.type>=CMD_RELEASE_HOPPER_LEFT&&c.type<=CMD_RELEASE_HOOK_RIGHT&&!stopped_){++mockReleases_;executed=true;}
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
private:
 SessionHmac mac_;SessionHandshake session_;RadioSessionCounter rx_;
 uint8_t root_[32]{42},key_[32]{};uint16_t sequence_=0,lastStop_=0;bool stopped_=false;unsigned mockReleases_=0;
};
}
