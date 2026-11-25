#include <cassert>
#include <cstdint>
#include <iostream>
#include "../source/premade_module_output.h"

static ModuleOnlyAcceptanceR15 qualified() {
    ModuleOnlyAcceptanceR15 q;
    q.premadeElectronicModulesOnly=true;
    q.pinoutsMeasured=true;
    q.fusesAndConductorsQualified=true;
    q.physicalEstopAndDcCutoffQualified=true;
    q.independentWatchdogQualified=true;
    q.nonretriggerableOneHotQualified=true;
    q.hardwareCurrentZeroQualified=true;
    q.powerOnAndBrownoutDefaultOffProven=true;
    q.moduleInputsDefaultLowMeasured=true;
    q.actualDummyLoadTestsPassed=true;
    return q;
}
static ModuleOnlyFeedbackR15 sample(uint32_t t) {
    ModuleOnlyFeedbackR15 f;
    f.sampledAtMs=t;f.trusted=true;f.batteryHealthy=true;
    f.physicalStopReleased=true;f.independentWatchdogHealthy=true;
    f.powerCutoffFeedbackOn=true;f.pulseTimerReady=true;
    f.motorCurrentZero=true;f.coilCurrentZero=true;
    f.powerRailsGood=true;f.faultPresent=false;
    return f;
}
int main(){
    PremadeModuleCommandBridgeR15 bridge;
    ModuleOnlyRequestR15 request;ModuleOnlyOutputR15 out;
    assert(!bridge.prepare(request,sample(1),1,out));
    assert(out.solenoidTriggerEdgeMask==0&&out.motorLeft==0);
    ModuleOnlyAcceptanceR15 proof;
    assert(!bridge.configure(proof));
    proof=qualified();
    for(int i=0;i<10;i++){
        ModuleOnlyAcceptanceR15 bad=proof;
        switch(i){
        case 0:bad.premadeElectronicModulesOnly=false;break;
        case 1:bad.pinoutsMeasured=false;break;
        case 2:bad.fusesAndConductorsQualified=false;break;
        case 3:bad.physicalEstopAndDcCutoffQualified=false;break;
        case 4:bad.independentWatchdogQualified=false;break;
        case 5:bad.nonretriggerableOneHotQualified=false;break;
        case 6:bad.hardwareCurrentZeroQualified=false;break;
        case 7:bad.powerOnAndBrownoutDefaultOffProven=false;break;
        case 8:bad.moduleInputsDefaultLowMeasured=false;break;
        case 9:bad.actualDummyLoadTestsPassed=false;break;
        }
        assert(!bridge.configure(bad));assert(bridge.faulted());
    }
    assert(bridge.configure(proof));
    request={};
    assert(bridge.prepare(request,sample(100),100,out));
    assert(bridge.prepare(request,sample(149),149,out));
    request.solenoidChannel=0;
    assert(bridge.prepare(request,sample(150),150,out));
    assert(out.solenoidTriggerEdgeMask==0); // not armed until 50ms zero
    request.solenoidChannel=-1;
    assert(bridge.prepare(request,sample(151),151,out));
    assert(bridge.prepare(request,sample(201),201,out));
    for(int channel=0;channel<4;channel++){
        request.solenoidChannel=(int8_t)channel;
        assert(bridge.prepare(request,sample(220+channel*200),220+channel*200,out));
        assert(out.solenoidTriggerEdgeMask==(1U<<channel));
        for(uint32_t i=0;i<100;i++){
            assert(bridge.prepare(request,sample(221+channel*200+i),
                221+channel*200+i,out));
            assert(out.solenoidTriggerEdgeMask==0); // held never repeats
        }
        request.solenoidChannel=-1;
        assert(bridge.prepare(request,sample(321+channel*200),321+channel*200,out));
        assert(bridge.prepare(request,sample(371+channel*200),371+channel*200,out));
    }
    // Two active 550 motors: allowed with zero coil current.
    request.motorLeft=500;request.motorRight=-400;
    assert(bridge.prepare(request,sample(1300),1300,out));
    assert(out.motorLeft==500&&out.motorRight==-400&&out.solenoidTriggerEdgeMask==0);
    // Simultaneous motor + hopper is a latched fault.
    request.solenoidChannel=1;
    assert(!bridge.prepare(request,sample(1301),1301,out));
    assert(bridge.faulted()&&out.motorLeft==0&&out.motorRight==0);
    assert(!bridge.prepare(ModuleOnlyRequestR15{},sample(1302),1302,out));
    assert(bridge.configure(proof));
    assert(bridge.prepare(ModuleOnlyRequestR15{},sample(2000),2001,out)); // valid after explicit fake test requalification
    return 0;
}
