#include "../source/mission_controller.h"
#include "../source/mission_stick.h"
#include "mission_control_client.h"
#include <cassert>
#include <iostream>
// Application-level integration, with authentication and physical inputs
// represented by explicit flags. This is not a live RF or motor test.
int main(){
 MissionPlan plan={2,0,0,0,300,0,1000,0,0};
 ExecInputs in={{0,0,1,90,true,true,true,true},true,true,true,true,true,true,false,false};
 BoatMissionController boat;MissionControlClient remote;MissionStickLease lease;
 uint8_t wire[8];MissionControlRequest req{},decoded{};MissionControlFeedback ack{},fb{};
 assert(boat.stage(plan,0,true,true));remote.bind(2);
 assert(remote.observe(boat.status(),1,true));
 assert(lease.receive({2,0,0,true},2,1,true));in.stopActive=false;
 assert(remote.request(MCTRL_START,2)&&remote.next(2,req));
 assert(missionControlEncode(req,wire)&&missionControlDecode(wire,decoded));
 assert(boat.request(decoded,in,2,true,true,lease.neutral(2,2),ack)&&ack.result==MC_ACCEPTED);
 // Lost ACK: same logical request, fresh envelope would be used by adapter.
 assert(remote.next(402,req)&&req.request==1);
 assert(boat.request(req,in,402,true,true,false,ack)&&ack.result==MC_ACCEPTED);
 assert(missionFeedbackEncode(ack,wire)&&missionFeedbackDecode(wire,fb));
 assert(remote.observe(fb,403,true)&&remote.phase()==MC_OUTBOUND);
 assert(remote.request(MCTRL_TEMP_OVERRIDE,404)&&remote.next(404,req));
 assert(boat.request(req,in,404,true,true,false,ack)&&ack.phase==MC_OVERRIDE_NEUTRAL);
 assert(remote.observe(ack,405,true));lease.reset();
 assert(boat.tick(in,406,true,lease.neutral(2,406)).intent==INTENT_STOP);
 assert(!boat.temporaryOverrideConfirmed());
 assert(lease.receive({2,0,0,true},2,407,true));boat.tick(in,407,true,lease.neutral(2,407));
 assert(remote.observe(boat.status(),408,true)&&remote.phase()==MC_OVERRIDE_ACTIVE);
 assert(lease.receive({2,100,100,false},2,409,true));
 int16_t l,r;assert(lease.output(2,608,true,l,r)&&l==100&&r==100);
 assert(!lease.output(2,609,true,l,r)&&l==0&&r==0);
 boat.emergencyAbort();lease.reset();assert(boat.tick(in,610,true,false).intent==INTENT_STOP);
 assert(remote.observe(boat.status(),611,true)&&!remote.locked());
 std::cout<<"Mission transport application integration passed\n";
}
