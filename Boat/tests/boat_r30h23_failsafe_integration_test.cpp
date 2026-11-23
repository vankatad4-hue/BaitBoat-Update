// R30H-23 - real Boat safety policies + mission/actuator output requests.
// Deterministic HOST simulation. No GPIO, no real sensor, no RF or motor feed.
// Physics inputs are deliberately injected, not inferred from a radio ACK.
#include "../source/boat_hardware_supervisor.h"
#include "../source/boat_hardware_status.h"
#include "../source/boat_shared_supply_fault_guard_r30b.h"
#include "../source/boat_home_point_record.h"
#include "../source/boat_radio_safety.h"
#include "../source/mission_controller.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

static bool off(BoatActuatorOutput o){
    return o.left==0 && o.right==0 && o.releaseChannel==-1;
}
static bool driving(BoatActuatorOutput o){
    return (o.left>0 || o.right>0) && o.left>=0 && o.right>=0 &&
           o.releaseChannel==-1;
}
struct Rig {
    static constexpr uint32_t AUTH_LAST_MS=1000U;
    MissionPlan plan={73,0,0,0,300,0,2000,0,0};
    BoatHomePoint home={0,0,1U};
    BoatActuatorSupervisor executor{};
    BoatMissionController mission{};
    BoatRadioWatchdog watchdog{};
    BoatBatteryMonitor battery{};
    BoatHardwareSample sample{};
    bool detached=false;
    bool authenticatedEver=true;
    uint32_t lastAuthenticatedAt=AUTH_LAST_MS;
    Rig(){
        assert(executor.configure({{120,140,160,180},400,3.0}));
        fresh(AUTH_LAST_MS);
        mission.resetSession();
        assert(mission.stage(plan,AUTH_LAST_MS,true,true));
        assert(executor.clearStop(sample,AUTH_LAST_MS,true,true));
        ExecInputs input=inputs(AUTH_LAST_MS,true,true);
        MissionControlFeedback ack{};
        assert(mission.request({MCTRL_START,plan.transaction,1},input,1001U,true,
                               true,true,ack));
        assert(ack.result==MC_ACCEPTED);
        assert(watchdog.update(true,true)==BOAT_RADIO_WATCHDOG_RESTORED);
    }
    void fresh(uint32_t now){
        sample.fix={0,1000,1.0,90.0,true,true,true,true};
        sample.safetyAt=now;sample.gnssAt=now;sample.headingAt=now;
        sample.safetyValid=true;sample.physicalStopReleased=true;
        sample.batteryHealthy=true;sample.stationary=true;
        sample.releaseInterlocks=true;sample.releaseIdle=true;
        sample.watchdogReady=true;sample.outputFeedbackValid=true;
        sample.leftMotorActive=false;sample.rightMotorActive=false;
        sample.batteryPercent=75;sample.batteryMillivolts=12000;
        sample.batteryAt=now;sample.voltageValid=true;
        sample.batteryBand=BOAT_BATTERY_GREEN;
        sample.batteryRthRequested=false;sample.gpsFixType=3;
        sample.satellites=9;
    }
    ExecInputs inputs(uint32_t now,bool primary,bool backup)const{
        ExecInputs in=boatHardware_inputs(sample,now,executor.stopped());
        in.primaryFresh=primary;in.backupFresh=backup;
        in.autonomousRth=detached;
        return in;
    }
    BoatActuatorOutput step(uint32_t now,bool primary,bool backup){
        const auto in=inputs(now,primary,backup);
        const ExecOutput intent=mission.tick(in,now,true,true);
        const auto nav=mission.navigation(in);
        const auto phase=mission.status().phase;
        const bool active=phase>=MC_PREPARED&&phase<=MC_RETURNING;
        return executor.tick(in,now,intent,nav,active,false,0,0,sample.safetyAt);
    }
    bool admitRadioLossHome(uint32_t now,bool primary,bool backup){
        const auto in=inputs(now,primary,backup);
        const bool matchingHome=boatHomePointValid(home)&&
            mission.homeMatches(home.latE7,home.lonE7);
        const bool navSafe=!in.stopActive&&in.releaseIdle&&
            in.releaseInterlocks&&mission.homeNavigationValid(in);
        if(!boatRadioLossRthDue(!primary&&!backup,mission.canDivertHome(),
                               matchingHome,navSafe,authenticatedEver,
                               lastAuthenticatedAt,now,10000U))return false;
        if(!mission.divertHome(in,true))return false;
        detached=true;
        return true;
    }
};

static void noLinkThenHome(){
    Rig r;
    r.fresh(1002U);
    assert(driving(r.step(1002U,true,true)));
    assert(r.watchdog.update(false,false)==BOAT_RADIO_WATCHDOG_LOST);
    r.fresh(2000U);
    assert(off(r.step(2000U,false,false)));
    assert(r.mission.status().phase==MC_OUTBOUND);
    r.fresh(10999U);
    assert(!r.admitRadioLossHome(10999U,false,false));
    assert(off(r.step(10999U,false,false)));
    assert(!r.detached);
    r.fresh(11000U);
    // The local Home is west of the simulated boat. Heading is updated by
    // a locally fresh fake compass, not a command or a stale RF packet.
    r.sample.fix.headingDeg=270.0;
    assert(r.admitRadioLossHome(11000U,false,false));
    assert(r.detached&&r.mission.status().phase==MC_RETURNING);
    r.fresh(11001U);r.sample.fix.headingDeg=270.0;
    assert(driving(r.step(11001U,false,false)));
    // RTH may not send a second hopper/hook pulse without primary link.
    const auto noRelease=r.inputs(11001U,false,false);
    assert(!r.executor.beginRelease(73,1,0,noRelease,11001U));
    // Restoration may continue the existing HOME-only plan, not outbound.
    assert(r.watchdog.update(true,false)==BOAT_RADIO_WATCHDOG_RESTORED);
    r.fresh(11002U);r.sample.fix.headingDeg=270.0;
    assert(driving(r.step(11002U,true,false)));
    assert(r.mission.status().phase==MC_RETURNING);
    // Navigation fixes and course must stay locally fresh DURING RTH.
    r.sample.safetyAt=13003U;r.sample.headingAt=13003U;
    r.sample.gnssAt=11002U; // only GNSS is stale here
    assert(off(r.step(13003U,false,false)));
    assert(r.mission.status().phase==MC_ABORTED);
    // A restored radio does NOT implicitly revive the aborted mission.
    r.fresh(13004U);
    assert(off(r.step(13004U,true,true)));
    assert(r.mission.status().phase==MC_ABORTED);
}
static void backupOnlyAndNoFirstHandshake(){
    Rig r;r.fresh(1200U);
    assert(r.watchdog.update(false,true)==BOAT_RADIO_WATCHDOG_NO_CHANGE);
    assert(driving(r.step(1200U,false,true)));
    auto in=r.inputs(1200U,false,true);
    assert(!r.executor.beginRelease(73,1,0,in,1200U));
    r.fresh(11000U);
    assert(!r.admitRadioLossHome(11000U,false,true)); // backup still fresh
    r.authenticatedEver=false;
    assert(!r.admitRadioLossHome(11000U,false,false));
    r.authenticatedEver=true;r.lastAuthenticatedAt=11000U;
    assert(!r.admitRadioLossHome(11000U,false,false)); // timer reset on Rx
    assert(!r.admitRadioLossHome(20999U,false,false));
    // Deadline arithmetic must work across ESP32 millis() wraparound.
    assert(boatRadioLossRthDue(true,true,true,true,true,0xFFFFFE00U,
           0xFFFFFE00U+10000U));
}
static void rejectUnsafeHome(){
    // Distinct locally sampled fault conditions must independently deny
    // the radio-less Home diversion after the 10-second timer has expired.
    for(unsigned which=0;which<15;++which){
        Rig r;r.fresh(2000U);
        assert(off(r.step(2000U,false,false)));
        r.fresh(11000U);r.sample.fix.headingDeg=270.0;
        switch(which){
          case 0: r.home.sequence=0;break;
          case 1: r.home.latE7=100;break;
          case 2: r.sample.fix.fresh=false;break;
          case 3: r.sample.gnssAt=9000U;break; // stale GNSS > 2 seconds
          case 4: r.sample.fix.headingValid=false;break;
          case 5: r.sample.headingAt=10699U;break; // heading age > 300 ms
          case 6: r.sample.fix.geofenceValid=false;break;
          case 7: r.sample.fix.accuracyM=std::numeric_limits<double>::quiet_NaN();break;
          case 8: r.sample.batteryHealthy=false;break;
          case 9: r.sample.physicalStopReleased=false;break;
          case 10:r.sample.releaseIdle=false;break;
          case 11:r.sample.releaseInterlocks=false;break;
          case 12:r.sample.watchdogReady=false;break;
          case 13:r.sample.outputFeedbackValid=false;break;
          case 14:r.sample.safetyAt=10899U;break; // stale safety <100 ms gate
        }
        assert(!r.admitRadioLossHome(11000U,false,false));
        assert(!r.detached);
        assert(off(r.step(11000U,false,false)));
    }
}
static void faultDuringHome(){
    for(unsigned fault=0;fault<7;++fault){
        Rig r;r.fresh(2000U);assert(off(r.step(2000U,false,false)));
        r.fresh(11000U);r.sample.fix.headingDeg=270;
        assert(r.admitRadioLossHome(11000U,false,false));
        r.fresh(11001U);r.sample.fix.headingDeg=270;
        assert(driving(r.step(11001U,false,false)));
        r.fresh(11002U);r.sample.fix.headingDeg=270;
        switch(fault){
          case 0:r.sample.fix.fresh=false;break;
          case 1:r.sample.headingAt=10000U;break;
          case 2:r.sample.fix.geofenceValid=false;break;
          case 3:r.sample.batteryHealthy=false;break;
          case 4:r.sample.physicalStopReleased=false;break;
          case 5:r.sample.releaseInterlocks=false;break;
          case 6:r.sample.outputFeedbackValid=false;break;
        }
        assert(off(r.step(11002U,false,false)));
        // No navigation was authorized from a failed local input.
        assert(r.mission.status().phase==MC_ABORTED ||
               !r.executor.stopped()); // interlock-only faults stop output
    }
}
static void energyAbortAndCriticalStop(){
    Rig r;r.fresh(1002U);assert(driving(r.step(1002U,true,true)));
    BoatBatterySnapshot snap=r.battery.update(true,10700,1002U);
    assert(snap.valid&&!snap.rthRequested&&snap.band==BOAT_BATTERY_RED);
    snap=r.battery.update(true,10700,4001U);
    assert(!snap.rthRequested);
    snap=r.battery.update(true,10700,4002U);
    assert(snap.rthRequested&&!snap.critical);
    auto energy=boat_r30b::inspectBoatBattery(snap,true,true,10700,4002U,4002U);
    assert(energy==boat_r30b::EnergyEvidence::LOW_BATTERY);
    r.fresh(4002U);r.sample.batteryBand=snap.band;
    r.sample.batteryRthRequested=snap.rthRequested;
    r.sample.fix.headingDeg=270.0;
    auto in=r.inputs(4002U,true,true);
    assert(in.batteryLow);
    assert(off(r.step(4002U,true,true))); // outbound held before Home
    assert(r.mission.status().phase==MC_OUTBOUND);
    assert(boat_r30b::canRequestConditionalHome(energy,
                 boatHomePointValid(r.home),r.mission.homeNavigationValid(in),
                 in.releaseIdle,r.sample.physicalStopReleased));
    assert(r.mission.divertHome(in,true));r.detached=true;
    r.fresh(4003U);r.sample.batteryBand=snap.band;
    r.sample.batteryRthRequested=true;r.sample.fix.headingDeg=270.0;
    assert(driving(r.step(4003U,true,true)));
    // Single abrupt 9.8V INA231 sample bypasses smoothing and STOPs.
    snap=r.battery.update(true,9800U,4004U);
    assert(snap.critical&&!snap.rthRequested);
    energy=boat_r30b::inspectBoatBattery(snap,true,true,9800U,4004U,4004U);
    assert(energy==boat_r30b::EnergyEvidence::STOP_INVALID_OR_CRITICAL);
    r.fresh(4004U);r.sample.batteryBand=snap.band;
    r.sample.batteryHealthy=false;
    assert(off(r.step(4004U,true,true)));
    assert(r.executor.stopped());
    assert(!boat_r30b::canRequestConditionalHome(energy,true,true,true,true));
    // Radio recovery or a healthy voltage alone may not clear the STOP latch.
    r.fresh(4005U);
    assert(off(r.step(4005U,true,true)));
    assert(r.executor.stopped());
    // Stale/invalid INA231 evidence independently denies any Home movement.
    BoatBatteryMonitor stale{};
    auto valid=stale.update(true,12000U,1000U);
    assert(valid.valid);
    auto absent=stale.snapshot(2001U);
    assert(!absent.valid);
    assert(boat_r30b::inspectBoatBattery(absent,true,true,12000U,
           1000U,2001U)==boat_r30b::EnergyEvidence::STOP_INVALID_OR_CRITICAL);
    absent=stale.update(false,0,2002U);
    assert(!absent.valid);
    assert(boat_r30b::inspectBoatBattery(absent,true,false,0,
           2002U,2002U)==boat_r30b::EnergyEvidence::STOP_INVALID_OR_CRITICAL);
}
static void interruptedCoilCannotReplay(){
    Rig r;r.fresh(1002U);
    auto in=r.inputs(1002U,true,true);
    assert(r.executor.beginRelease(73,1,0,in,1002U));
    assert(r.executor.tick(in,1003U,{INTENT_RELEASE,0,1},{},
             true,false,0,0,1003U).releaseChannel==0);
    r.fresh(1004U);
    in=r.inputs(1004U,false,false);
    const auto offResult=r.executor.tick(in,1004U,{INTENT_STOP,-1,0},{},
                                     true,false,0,0,1004U);
    assert(off(offResult)&&r.executor.stopped());
    const auto completion=r.executor.takeCompletion();
    assert(completion.valid&&!completion.success);
    r.fresh(1005U);
    in=r.inputs(1005U,true,true);
    assert(off(r.executor.tick(in,1005U,{INTENT_RELEASE,0,1},{},
                 true,false,0,0,1005U)));
    assert(!r.executor.beginRelease(73,1,0,in,1005U));
    // Even 10 seconds later, an actuator-latched STOP cannot authorize RTH.
    r.fresh(11000U);r.sample.fix.headingDeg=270.0;
    assert(!r.admitRadioLossHome(11000U,false,false));
}
static void statusNeedsRealFeedback(){
    Rig r;r.fresh(1000U);
    r.executor.stop();
    auto sample=boatHardware_status(r.sample,1000U,r.executor.stopped(),
                                    MC_ABORTED,false,true);
    assert(sample.stopState==1);
    uint8_t wire[8]{};
    assert(radio_encodeTelemetryStatusPayload(sample.boatMode,sample.stopState,
      sample.safetyFlags,sample.batteryPercent,sample.gpsFixType,
      sample.satellites,wire));
    RadioDecodedPayload observed{};
    assert(radio_decodeTelemetryPayload(wire,observed));
    assert(observed.stopState==1U);
    r.sample.leftMotorActive=true;
    sample=boatHardware_status(r.sample,1000U,true,MC_ABORTED,false,true);
    assert(sample.stopState==2U); // motor not stationary
    r.sample.leftMotorActive=false;r.sample.outputFeedbackValid=false;
    sample=boatHardware_status(r.sample,1000U,true,MC_ABORTED,false,true);
    assert(sample.stopState==2U);
    r.sample.outputFeedbackValid=true;r.sample.watchdogReady=false;
    sample=boatHardware_status(r.sample,1000U,true,MC_ABORTED,false,true);
    assert(sample.stopState==2U);
}
int main(){
    noLinkThenHome();
    backupOnlyAndNoFirstHandshake();
    rejectUnsafeHome();
    faultDuringHome();
    energyAbortAndCriticalStop();
    interruptedCoilCannotReplay();
    statusNeedsRealFeedback();
    std::cout<<"R30H-23 dual-link loss / conditional 10-second Home / local safety / battery / coil faults PASS (host-only)\n";
}
