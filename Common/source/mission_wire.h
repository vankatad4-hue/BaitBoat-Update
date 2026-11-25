#pragma once
#include "mission_protocol.h"
// Separate preparation acknowledgement; never an actuator execution ACK.
// Wire: transaction LE16, opcode, result, reserved zero[4].
inline bool missionAck_encode(uint16_t id,MissionOpcode op,MissionReceive result,uint8_t out[8]){
 if(!out||!id||op<MISSION_BEGIN||op>MISSION_CANCEL||result>MISSION_CANCELLED)return false;
 out[0]=(uint8_t)id;out[1]=(uint8_t)(id>>8);out[2]=(uint8_t)op;out[3]=(uint8_t)result;
 for(uint8_t i=4;i<8;++i)out[i]=0;
 return true;
}
inline bool missionAck_decode(const uint8_t in[8],uint16_t &id,MissionOpcode &op,MissionReceive &result){
 if(!in)return false;
 const uint16_t tx=(uint16_t)in[0]|((uint16_t)in[1]<<8);
 if(!tx||in[2]<MISSION_BEGIN||in[2]>MISSION_CANCEL||in[3]>MISSION_CANCELLED)return false;
 for(uint8_t i=4;i<8;++i)if(in[i])return false;
 id=tx;op=(MissionOpcode)in[2];result=(MissionReceive)in[3];return true;
}
