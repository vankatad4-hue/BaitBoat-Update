#pragma once
#include <stdint.h>
// Pure feedback state machine, never drives motors or actuators.
enum MissionPhase:uint8_t{MISSION_IDLE=0,MISSION_UPLOADING,MISSION_READY,MISSION_OUTBOUND,MISSION_PAUSED,MISSION_RELEASING,MISSION_RETURNING,MISSION_COMPLETE,MISSION_STOP_REQUIRED,MISSION_ABORTED};
class MissionLifecycle{
public:
 MissionLifecycle():phase_(MISSION_IDLE),id_(0),last_(0){}
 bool upload(uint16_t id,uint32_t now);
 bool observe(uint16_t id,MissionPhase next,uint32_t now,bool authenticated);
 void tick(uint32_t now,bool linkFresh,bool navigationValid);
 void stop(){if(phase_!=MISSION_IDLE&&phase_!=MISSION_COMPLETE&&phase_!=MISSION_ABORTED)phase_=MISSION_STOP_REQUIRED;}
 MissionPhase phase()const{return phase_;}
 uint16_t transaction()const{return id_;}
private:MissionPhase phase_;uint16_t id_;uint32_t last_;
};
