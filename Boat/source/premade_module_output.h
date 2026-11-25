#pragma once
#include <stdint.h>
// R15 PREMADE MODULE-ONLY OUTPUT ADAPTER CONTRACT, host-verifiable only.
// The module below is NOT a GPIO driver, does NOT implement an independent
// hardware cutoff, and MUST NOT be used to energize motors or coils until the
// off-the-shelf STOP, watchdog, pulse timer, current-zero sense, and connector
// chain are accepted on real hardware.
struct ModuleOnlyAcceptanceR15 {
    bool premadeElectronicModulesOnly=false;
    bool pinoutsMeasured=false;
    bool fusesAndConductorsQualified=false;
    bool physicalEstopAndDcCutoffQualified=false;
    bool independentWatchdogQualified=false;
    bool nonretriggerableOneHotQualified=false;
    bool hardwareCurrentZeroQualified=false;
    bool powerOnAndBrownoutDefaultOffProven=false;
    bool moduleInputsDefaultLowMeasured=false;
    bool actualDummyLoadTestsPassed=false;
    bool all() const {
        return premadeElectronicModulesOnly&&pinoutsMeasured&&
            fusesAndConductorsQualified&&physicalEstopAndDcCutoffQualified&&
            independentWatchdogQualified&&nonretriggerableOneHotQualified&&
            hardwareCurrentZeroQualified&&powerOnAndBrownoutDefaultOffProven&&
            moduleInputsDefaultLowMeasured&&actualDummyLoadTestsPassed;
    }
};
struct ModuleOnlyFeedbackR15 {
    uint32_t sampledAtMs=0;
    bool trusted=false;
    bool batteryHealthy=false;
    bool physicalStopReleased=false;
    bool independentWatchdogHealthy=false;
    bool powerCutoffFeedbackOn=false;
    bool pulseTimerReady=false;
    bool motorCurrentZero=false;
    bool coilCurrentZero=false;
    bool powerRailsGood=false;
    bool faultPresent=true;
};
struct ModuleOnlyRequestR15 {
    int16_t motorLeft=0; // normalized -1000..1000, not a physical PWM
    int16_t motorRight=0;
    int8_t solenoidChannel=-1; // -1=none, 0=LH, 1=RH, 2=LK, 3=RK
};
struct ModuleOnlyOutputR15 {
    int16_t motorLeft=0;
    int16_t motorRight=0;
    uint8_t solenoidTriggerEdgeMask=0; // pulse request; downstream HW timer required
};
class PremadeModuleCommandBridgeR15 {
public:
    bool configure(const ModuleOnlyAcceptanceR15 &proof) {
        fault();
        commissioned_=proof.all();
        if(commissioned_)latchedFault_=false;
        return commissioned_;
    }
    void fault() {
        commissioned_=false;
        latchedFault_=true;
        armedRelease_=false;
        idleActive_=false;
    }
    bool faulted() const { return latchedFault_ || !commissioned_; }
    // This gate is additional SOFTWARE defense only. Physical safety must
    // operate independently even if the MCU stops refreshing this function.
    bool prepare(const ModuleOnlyRequestR15 &req,
                 const ModuleOnlyFeedbackR15 &fb, uint32_t now,
                 ModuleOnlyOutputR15 &out) {
        out={};
        if(faulted() || !fb.trusted || now<fb.sampledAtMs ||
           now-fb.sampledAtMs>=50U || !fb.batteryHealthy ||
           !fb.physicalStopReleased || !fb.independentWatchdogHealthy ||
           !fb.powerCutoffFeedbackOn || !fb.powerRailsGood ||
           fb.faultPresent || req.motorLeft < -1000 ||
           req.motorLeft > 1000 || req.motorRight < -1000 ||
           req.motorRight > 1000 || req.solenoidChannel < -1 ||
           req.solenoidChannel > 3) {
            fault();
            return false;
        }
        if(req.solenoidChannel>=0) {
            if(req.motorLeft || req.motorRight || !fb.motorCurrentZero ||
               !fb.pulseTimerReady) {
                fault();
                return false;
            }
            // Software edge one-shot only. The independent premade timer
            // MUST clamp actual energized duration and fail OFF on held HIGH.
            if(armedRelease_ && fb.coilCurrentZero) {
                out.solenoidTriggerEdgeMask =
                    static_cast<uint8_t>(1U << req.solenoidChannel);
                armedRelease_=false;
            }
            idleActive_=false;
            return true; // held command produces NO more trigger edges
        }
        // A fresh continuous observed-zero interval is required to rearm.
        if(req.motorLeft || req.motorRight || !fb.coilCurrentZero ||
           !fb.motorCurrentZero) {
            armedRelease_=false;
            idleActive_=false;
            if(!fb.coilCurrentZero) {
                // Motors may never be enabled while coil current is flowing.
                if(req.motorLeft || req.motorRight) { fault(); return false; }
            }
        } else {
            if(!idleActive_) {
                idleSince_=now;
                idleActive_=true;
            } else if(now-idleSince_>=50U) {
                armedRelease_=true;
            }
        }
        // In software the prebuilt Cytron MDD20A is still controlled by the
        // existing motor profile; this adapter is only an upstream request.
        if(!fb.coilCurrentZero) return true;
        out.motorLeft=req.motorLeft;
        out.motorRight=req.motorRight;
        return true;
    }
private:
    bool commissioned_=false,latchedFault_=true,armedRelease_=false;
    bool idleActive_=false;
    uint32_t idleSince_=0;
};
