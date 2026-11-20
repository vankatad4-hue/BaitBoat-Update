#pragma once
#include "boat_hardware_supervisor.h"
#include "mission_control_wire.h"
#include "radio_payload_codec.h"
// Status uses locally observed feedback, never a command's requested output.
inline RadioDecodedPayload boatHardware_status(const BoatHardwareSample &s,uint32_t now,bool stopped,MissionControlPhase phase,bool detached,bool primary){
 RadioDecodedPayload out{};out.kind=RADIO_PAYLOAD_STATUS;out.stopState=2;out.boatMode=3;out.batteryPercent=255;
 const auto in=boatHardware_inputs(s,now,false);
 if(!s.safetyValid||!s.outputFeedbackValid||now-s.safetyAt>=100U||!s.watchdogReady)return out;
 if(stopped||!s.physicalStopReleased){if(in.stationary&&in.releaseIdle)out.stopState=1;}
 else if(!in.stopActive&&in.fix.batteryHealthy)out.stopState=0;
 if(phase==MC_RETURNING)out.boatMode=2;
 else if(phase>=MC_OUTBOUND&&phase<=MC_RELEASING)out.boatMode=1;
 else if(phase==MC_PREPARED||phase==MC_IDLE||phase==MC_COMPLETE||phase==MC_ABORTED){
  if(in.stationary&&in.releaseIdle)out.boatMode=0;
  else if(phase==MC_IDLE&&!stopped)out.boatMode=0;
 }
 if(out.stopState==0&&in.releaseInterlocks&&primary&&!detached&&out.boatMode!=3)out.safetyFlags|=1U;
 if(s.leftMotorActive)out.safetyFlags|=2U;
 if(s.rightMotorActive)out.safetyFlags|=4U;
 if(in.fix.fresh&&std::isfinite(in.fix.accuracyM)&&in.fix.accuracyM>0&&s.gpsFixType>=3){out.safetyFlags|=8U;out.gpsFixType=s.gpsFixType;out.satellites=s.satellites;}
 if(in.fix.headingValid&&std::isfinite(in.fix.headingDeg)&&in.fix.headingDeg>=0&&in.fix.headingDeg<360)out.safetyFlags|=16U;
 if(!in.fix.batteryHealthy)out.safetyFlags|=32U;
 if(detached||!primary||out.boatMode==3)out.safetyFlags|=64U;
 out.batteryPercent=s.batteryPercent<=100?s.batteryPercent:255;
 return out;
}
