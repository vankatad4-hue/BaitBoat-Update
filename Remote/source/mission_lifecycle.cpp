#include "mission_lifecycle.h"
bool MissionLifecycle::upload(uint16_t id,uint32_t now){
 if(!id||(phase_!=MISSION_IDLE&&phase_!=MISSION_COMPLETE&&phase_!=MISSION_ABORTED))return false;
 if(id==id_)return false;
 id_=id;phase_=MISSION_UPLOADING;last_=now;return true;
}
bool MissionLifecycle::observe(uint16_t id,MissionPhase next,uint32_t now,bool auth){
 if(!auth||id!=id_||!id||next>MISSION_ABORTED)return false;
 if(next==phase_){last_=now;return true;}
 bool ok=false;
 if(next==MISSION_ABORTED&&phase_!=MISSION_IDLE&&phase_!=MISSION_COMPLETE)ok=true;
 else switch(phase_){
 case MISSION_UPLOADING:ok=next==MISSION_READY;break;
 case MISSION_READY:ok=next==MISSION_OUTBOUND;break;
 case MISSION_OUTBOUND:ok=next==MISSION_PAUSED||next==MISSION_RELEASING||next==MISSION_RETURNING;break;
 case MISSION_PAUSED:ok=next==MISSION_OUTBOUND||next==MISSION_RETURNING;break;
 case MISSION_RELEASING:ok=next==MISSION_RETURNING;break;
 case MISSION_RETURNING:ok=next==MISSION_COMPLETE;break;
 default:break;
 }
 if(!ok)return false;
 phase_=next;last_=now;return true;
}
void MissionLifecycle::tick(uint32_t now,bool linkFresh,bool navigationValid){
 if(phase_==MISSION_IDLE||phase_==MISSION_COMPLETE||phase_==MISSION_ABORTED||phase_==MISSION_STOP_REQUIRED)return;
 uint32_t limit=phase_==MISSION_UPLOADING?10000U:3000U;
 if(!linkFresh||now-last_>=limit||((phase_==MISSION_OUTBOUND||phase_==MISSION_RELEASING||phase_==MISSION_RETURNING)&&!navigationValid))stop();
}
