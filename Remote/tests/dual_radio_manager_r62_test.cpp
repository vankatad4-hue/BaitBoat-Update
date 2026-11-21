#include <cassert>
#include "../source/dual_radio_manager.h"
static bool transmitOK=false;
static ManagedLink lastLink=LINK_NONE;
static BoatCommand lastCommand{};
static unsigned sendCalls=0;
static bool sendMock(ManagedLink link,const BoatCommand &cmd){
    ++sendCalls;lastLink=link;lastCommand=cmd;return transmitOK;
}
int main(){
    DualRadioManager mgr;
    mgr.begin(0);
    // Invalid identifiers must not accidentally mark backup transport healthy.
    mgr.setTransportReady(LINK_NONE,true);
    mgr.reportValidRx(LINK_NONE,true,100);
    mgr.reportFailure(LINK_NONE);
    mgr.tick(100);
    assert(mgr.state()==LINK_NO_LINK);
    assert(!mgr.backupHealthy(100));

    // Force authenticated backup as the only available link.
    mgr.setTransportReady(LINK_LORA_BACKUP,true);
    mgr.reportValidRx(LINK_LORA_BACKUP,true,200);
    mgr.tick(200);
    assert(mgr.state()==LINK_BACKUP && mgr.active()==LINK_LORA_BACKUP);
    BoatCommand steering{};
    steering.type=CMD_MANUAL_MOTORS;
    steering.leftMotor=900;
    steering.rightMotor=-900;
    assert(!mgr.send(steering,sendMock,3000)); // simulated UART failure
    assert(sendCalls==1 && lastLink==LINK_LORA_BACKUP);
    assert(lastCommand.leftMotor==250 && lastCommand.rightMotor==-250);
    transmitOK=true;
    assert(mgr.send(steering,sendMock,3001)); // retry allowed after failure
    assert(sendCalls==2);
    assert(!mgr.send(steering,sendMock,3002)); // sent command is paced
    assert(sendCalls==2);
    assert(mgr.send(steering,sendMock,6001)); // 3 seconds have elapsed
    assert(sendCalls==3);
    assert(!mgr.send(steering,sendMock,6201)); // no fresh authenticated backup RX
    assert(sendCalls==3);
    BoatCommand release{};
    release.type=CMD_RELEASE_HOOK_LEFT;release.activate=true;
    assert(!mgr.send(release,sendMock,6002)); // no releases on backup

    // A healthy primary requires the authenticated recovery streak and holdoff.
    mgr.setTransportReady(LINK_NRF_PRIMARY,true);
    mgr.reportValidRx(LINK_NRF_PRIMARY,true,7000);
    mgr.tick(7000);
    assert(mgr.state()==LINK_RECOVERY);
    for(unsigned i=0;i<7;++i)mgr.reportValidRx(LINK_NRF_PRIMARY,true,8498);
    mgr.tick(8498);
    assert(mgr.state()==LINK_RECOVERY);
    mgr.reportValidRx(LINK_NRF_PRIMARY,true,8501);
    mgr.tick(8501);
    assert(mgr.state()==LINK_PRIMARY);
    assert(mgr.active()==LINK_NRF_PRIMARY);
}
