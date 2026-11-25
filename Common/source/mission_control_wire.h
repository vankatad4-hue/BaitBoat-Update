#pragma once
#ifndef FLYTEC_MISSION_CONTROL_WIRE_SHARED_H
#define FLYTEC_MISSION_CONTROL_WIRE_SHARED_H
#include <stdint.h>
// Session-authenticated mission-control contract, separate from plan upload.
// Request: op, transaction LE16, request ID LE16, three reserved zero bytes.
enum MissionControlOp:uint8_t{MCTRL_START=1,MCTRL_PAUSE,MCTRL_RESUME,MCTRL_CANCEL,MCTRL_TEMP_OVERRIDE,MCTRL_SWITCH_MANUAL};
enum MissionControlPhase:uint8_t{MC_IDLE,MC_PREPARED,MC_OUTBOUND,MC_PAUSED,MC_OVERRIDE_NEUTRAL,MC_OVERRIDE_ACTIVE,MC_RELEASING,MC_RETURNING,MC_COMPLETE,MC_ABORTED,MC_MANUAL_REQUESTED};
enum MissionControlResult:uint8_t{MC_STATUS=0,MC_ACCEPTED,MC_REJECTED};
struct MissionControlRequest{MissionControlOp op;uint16_t transaction,request;};
struct MissionControlFeedback{uint16_t transaction;MissionControlPhase phase;uint8_t op;uint16_t request;MissionControlResult result;};
inline void mcPut16(uint8_t *p,uint16_t n){p[0]=(uint8_t)n;p[1]=(uint8_t)(n>>8);}
inline uint16_t mcGet16(const uint8_t *p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
inline bool missionControlEncode(const MissionControlRequest &r,uint8_t p[8]){
 if(!p||r.op<MCTRL_START||r.op>MCTRL_SWITCH_MANUAL||!r.transaction||!r.request)return false;
 p[0]=(uint8_t)r.op;mcPut16(p+1,r.transaction);mcPut16(p+3,r.request);p[5]=p[6]=p[7]=0;return true;
}
inline bool missionControlDecode(const uint8_t p[8],MissionControlRequest &r){
 if(!p||p[0]<MCTRL_START||p[0]>MCTRL_SWITCH_MANUAL||p[5]||p[6]||p[7]||!mcGet16(p+1)||!mcGet16(p+3))return false;
 r={(MissionControlOp)p[0],mcGet16(p+1),mcGet16(p+3)};return true;
}
// Feedback: transaction LE16, phase, op, request LE16, result, zero.
// Status-only feedback has op/request zero; ACK must match all request fields.
inline bool missionFeedbackValid(const MissionControlFeedback &f){
 if(!f.transaction||f.phase>MC_MANUAL_REQUESTED||f.result>MC_REJECTED)return false;
 if(f.result==MC_STATUS)return !f.op&&!f.request;
 return f.op>=MCTRL_START&&f.op<=MCTRL_SWITCH_MANUAL&&f.request!=0;
}
inline bool missionFeedbackEncode(const MissionControlFeedback &f,uint8_t p[8]){
 if(!p||!missionFeedbackValid(f))return false;
 mcPut16(p,f.transaction);p[2]=(uint8_t)f.phase;p[3]=f.op;mcPut16(p+4,f.request);p[6]=(uint8_t)f.result;p[7]=0;return true;
}
inline bool missionFeedbackDecode(const uint8_t p[8],MissionControlFeedback &f){
 if(!p||p[7])return false;
 MissionControlFeedback v={mcGet16(p),(MissionControlPhase)p[2],p[3],mcGet16(p+4),(MissionControlResult)p[6]};
 if(!missionFeedbackValid(v))return false;
 f=v;return true;
}

#endif // FLYTEC_MISSION_CONTROL_WIRE_SHARED_H
