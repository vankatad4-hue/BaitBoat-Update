#include "mission_protocol.h"
#include "mission_lifecycle.h"
#include <cassert>
#include <iostream>
int main(){
 MissionPlan p={15,7,15,0x0F,300,430000000,250000000,430001000,250001000};
 assert(missionPlan_valid(p));uint8_t wire[8];MissionAssembler a;MissionPlan got={};
 assert(missionPlan_encode(p,MISSION_BEGIN,wire));assert(a.ingest(wire,1,false,true)==MISSION_REJECTED);assert(a.ingest(wire,1,true,false)==MISSION_REJECTED);
 assert(a.ingest(wire,1,true,true)==MISSION_PARTIAL);assert(a.ingest(wire,2,true,true)==MISSION_DUPLICATE);
 assert(missionPlan_encode(p,MISSION_COMMIT,wire));assert(a.ingest(wire,2,true,true)==MISSION_REJECTED);
 for(uint8_t op=2;op<=5;++op){assert(missionPlan_encode(p,(MissionOpcode)op,wire));assert(a.ingest(wire,3+op,true,true)==MISSION_PARTIAL);}
 assert(missionPlan_encode(p,MISSION_TARGET_LAT,wire));wire[4]^=1;assert(a.ingest(wire,9,true,true)==MISSION_REJECTED);
 assert(missionPlan_encode(p,MISSION_COMMIT,wire));wire[4]^=1;assert(a.ingest(wire,10,true,true)==MISSION_REJECTED);
 assert(missionPlan_encode(p,MISSION_COMMIT,wire));assert(a.ingest(wire,11,true,true)==MISSION_PREPARED);assert(a.ingest(wire,12,true,true)==MISSION_DUPLICATE);
 assert(a.getPrepared(got,12)&&missionPlan_digest(got)==missionPlan_digest(p));
 assert(missionPlan_encode(p,MISSION_CANCEL,wire));assert(a.ingest(wire,13,true,false)==MISSION_CANCELLED);assert(!a.getPrepared(got,14));
 assert(missionPlan_encode(p,MISSION_BEGIN,wire));assert(a.ingest(wire,15,true,true)==MISSION_REJECTED);
 ++p.transaction;assert(missionPlan_encode(p,MISSION_BEGIN,wire));assert(a.ingest(wire,20,true,true)==MISSION_PARTIAL);assert(missionPlan_encode(p,MISSION_HOME_LAT,wire));assert(a.ingest(wire,10020,true,true)==MISSION_REJECTED);
 p.releaseMask=16;assert(!missionPlan_valid(p));p.releaseMask=0;p.targetLatE7=910000000;assert(!missionPlan_valid(p));
 MissionLifecycle l;assert(l.upload(15,0));assert(!l.observe(15,MISSION_READY,1,false));assert(!l.observe(16,MISSION_READY,1,true));assert(!l.observe(15,MISSION_COMPLETE,1,true));
 assert(l.observe(15,MISSION_READY,2,true));assert(l.observe(15,MISSION_OUTBOUND,3,true));assert(l.observe(15,MISSION_PAUSED,4,true));assert(l.observe(15,MISSION_OUTBOUND,5,true));assert(l.observe(15,MISSION_RELEASING,6,true));assert(l.observe(15,MISSION_RETURNING,7,true));assert(l.observe(15,MISSION_COMPLETE,8,true));assert(l.upload(16,9));assert(l.observe(16,MISSION_READY,10,true));assert(l.observe(16,MISSION_OUTBOUND,11,true));l.tick(12,true,false);assert(l.phase()==MISSION_STOP_REQUIRED);assert(!l.observe(16,MISSION_OUTBOUND,13,true));assert(l.observe(16,MISSION_ABORTED,14,true));
 std::cout<<"Mission transaction and feedback state simulation passed\n";
}
