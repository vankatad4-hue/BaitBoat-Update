#include "remote_ordinary_intent.h"
#include <cassert>
using namespace flytec_rtos;
using S=OrdinaryIntent::State;
int main(){
 OrdinaryAuthorization a{};BoatCommand c{};OrdinaryIntent slot;
 c.type=CMD_MANUAL_MOTORS;c.commandId=1;c.createdAtMs=100;c.leftMotor=300;
 assert(!slot.stage(c,a,100));
 a.primaryFresh=a.sessionReady=a.stopConfirmed=a.manualConfirmed=a.actuatorReady=a.joystickValid=true;
 a.stopped=a.missionLocked=a.failsafe=false;
 assert(slot.stage(c,a,100));assert(!slot.stage(c,a,101));
 c.commandId=2;c.createdAtMs=110;c.leftMotor=0;assert(slot.stage(c,a,110));
 assert(slot.command().commandId==2&&slot.stillAllowed(a,159)&&!slot.stillAllowed(a,160));
 assert(slot.discardStaged());assert(!slot.stage(c,a,111));
 c.commandId=3;c.createdAtMs=200;assert(slot.stage(c,a,200));
 assert(slot.submitted(11,2,8,1));assert(!slot.discardStaged());
 a.primaryFresh=false;assert(!slot.stillAllowed(a,201));a.primaryFresh=true;
 assert(!slot.complete(11,1,3,8,1,200,50,true,202));
 assert(!slot.complete(11,2,3,8,1,201,50,true,202));
 assert(slot.complete(11,2,3,8,1,200,50,true,202)&&slot.state()==S::LocalEvidence);
 c.type=CMD_RELEASE_HOPPER_LEFT;c.activate=true;c.commandId=4;c.createdAtMs=300;
 assert(slot.stage(c,a,300));c.commandId=5;assert(!slot.stage(c,a,301));
 assert(slot.submitted(12,2,9,1));slot.transportRevoked();
 assert(!slot.complete(12,2,4,9,1,300,100,true,302));
 c.commandId=4;assert(!slot.stage(c,a,302));
 c.commandId=5;assert(slot.stage(c,a,302));assert(slot.submitted(13,3,10,1));
 assert(slot.complete(13,3,5,10,1,300,100,true,400)&&slot.state()==S::Unknown);
 c.type=CMD_CLEAR_STOP;c.activate=false;c.commandId=6;c.createdAtMs=400;a.stopped=true;
 assert(!slot.stage(c,a,400));c.type=CMD_SET_MODE;assert(slot.stage(c,a,400));
 slot.transportRevoked();c.type=CMD_BAIT_RELEASE;c.activate=true;c.commandId=7;a.stopped=false;
 assert(!slot.stage(c,a,400));c.type=CMD_SET_MODE;c.activate=false;c.mode=MODE_RETURN_HOME;
 assert(!slot.stage(c,a,400));
 slot.reset();c.mode=MODE_MANUAL;c.commandId=65535;c.createdAtMs=UINT32_MAX-10;
 assert(slot.stage(c,a,5));assert(slot.discardStaged());c.commandId=1;assert(slot.stage(c,a,5));
}
