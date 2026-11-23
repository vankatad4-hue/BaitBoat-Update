#include <cassert>
#include "../source/dual_radio_manager.h"
static bool transmitOK=true;
static ManagedLink lastLink=LINK_NONE;
static BoatCommand lastCommand{};
static unsigned sends=0;
static bool sendMock(ManagedLink link,const BoatCommand &cmd){
    ++sends;lastLink=link;lastCommand=cmd;return transmitOK;
}
int main(){
    DualRadioManager mgr;
    mgr.begin(0);
    mgr.setTransportReady(LINK_NONE,true);
    mgr.reportValidRx(LINK_NONE,true,100);
    mgr.reportFailure(LINK_NONE);
    mgr.tick(100);
    assert(mgr.state()==LINK_NO_LINK && !mgr.backupHealthy(100));

    mgr.setTransportReady(LINK_LORA_BACKUP,true);
    mgr.reportValidRx(LINK_LORA_BACKUP,true,200);
    mgr.tick(200);
    assert(mgr.state()==LINK_BACKUP && mgr.active()==LINK_LORA_BACKUP);
    BoatCommand steering{};steering.type=CMD_MANUAL_MOTORS;
    steering.leftMotor=900;steering.rightMotor=-900;
    assert(!mgr.send(steering,sendMock,3000)); // 200-ms motor lease cannot use 3-s backup pacing
    assert(sends==0);
    BoatCommand release{};release.type=CMD_RELEASE_HOOK_LEFT;release.activate=true;
    assert(mgr.send(release,sendMock,3001)); // discrete release, subject to Boat interlocks
    assert(sends==1 && lastLink==LINK_LORA_BACKUP);
    BoatCommand stop{};stop.type=CMD_EMERGENCY_STOP;
    transmitOK=false;
    assert(!mgr.send(stop,sendMock,3002)); // transport refusal never means delivered
    assert(sends==2);
    transmitOK=true;
    assert(mgr.send(stop,sendMock,3003));
    assert(sends==3 && lastLink==LINK_LORA_BACKUP &&
           lastCommand.type==CMD_EMERGENCY_STOP);
    BoatCommand home{};home.type=CMD_SET_MODE;home.mode=MODE_RETURN_HOME;
    assert(mgr.send(home,sendMock,3004));
    home.mode=MODE_AUTOPILOT;
    assert(mgr.send(home,sendMock,3005)); // allowed mode, not mission START authorization
    BoatCommand clear{};clear.type=CMD_CLEAR_STOP;
    assert(!mgr.send(clear,sendMock,3006));
    assert(sends==5);
    assert(!mgr.send(stop,sendMock,6201)); // backup freshness expires at 6000ms
    assert(sends==5);
    mgr.tick(6201);
    assert(mgr.state()==LINK_NO_LINK);
    mgr.reportValidRx(LINK_LORA_BACKUP,true,12000);
    mgr.tick(12000);
    assert(mgr.state()==LINK_BACKUP && mgr.active()==LINK_LORA_BACKUP);
    assert(mgr.send(stop,sendMock,12001));
    assert(sends==6);

    mgr.setTransportReady(LINK_NRF_PRIMARY,true);
    mgr.reportValidRx(LINK_NRF_PRIMARY,true,13000);
    mgr.tick(13000);
    assert(mgr.state()==LINK_RECOVERY);
    assert(!mgr.send(steering,sendMock,13000)); // no manual movement during recovery
    for(unsigned i=0;i<7;++i)mgr.reportValidRx(LINK_NRF_PRIMARY,true,14498);
    mgr.tick(14498);
    assert(mgr.state()==LINK_RECOVERY);
    mgr.reportValidRx(LINK_NRF_PRIMARY,true,14501);
    mgr.tick(14501);
    assert(mgr.state()==LINK_PRIMARY && mgr.active()==LINK_NRF_PRIMARY);
    assert(mgr.send(steering,sendMock,14501));
    assert(sends==7 && lastLink==LINK_NRF_PRIMARY &&
           lastCommand.leftMotor==900);
    mgr.reportFailure(LINK_NRF_PRIMARY);
    mgr.tick(14502);
    assert(mgr.state()==LINK_DEGRADED);
    assert(!mgr.send(steering,sendMock,14502));
    mgr.tick(14503);
    assert(mgr.state()==LINK_BACKUP && mgr.active()==LINK_LORA_BACKUP);
    assert(!mgr.send(steering,sendMock,14503));
    assert(mgr.send(stop,sendMock,14503));
    assert(sends==8 && lastLink==LINK_LORA_BACKUP);
}
