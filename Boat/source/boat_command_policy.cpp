#include "boat_command_policy.h"

static bool navReady(const BoatPolicyInputs &in) {
    return in.gnssFresh && in.headingFresh && in.geofenceValid && in.homeValid;
}
static int16_t limitMotor(int16_t value,int16_t limit) {
    if(value>limit) return limit;
    if(value<-limit) return (int16_t)-limit;
    return value;
}
void boatPolicyInit(BoatPolicyState &state) {
    state.mode=MODE_MANUAL;
    state.stopLatched=true; // fail-closed boot state
    state.missionActive=false;
}
static void safeOutput(BoatPolicyOutput &out) {
    out={}; out.releaseChannel=-1;
}
BoatPolicyResult boatPolicyApply(const BoatCommand &cmd,bool backup,
                                 const BoatPolicyInputs &in,
                                 BoatPolicyState &state,BoatPolicyOutput &out) {
    safeOutput(out);
    if(cmd.type==CMD_EMERGENCY_STOP) {
        state.stopLatched=true;
        state.missionActive=false;
        state.mode=MODE_MANUAL;
        return BOAT_POLICY_ACCEPTED;
    }
    if(cmd.type==CMD_CLEAR_STOP) {
        if(backup || !in.primaryLinkFresh || !in.physicalStopReleased ||
           !in.batteryHealthy || !in.motorsNeutral) return BOAT_POLICY_REJECTED_SAFETY;
        state.stopLatched=false;
        return BOAT_POLICY_ACCEPTED;
    }
    if(state.stopLatched || !in.physicalStopReleased || !in.batteryHealthy)
        return BOAT_POLICY_REJECTED_SAFETY;
    if(cmd.type==CMD_SET_MODE) {
        if(cmd.mode==MODE_MANUAL) {
            if(backup || !in.primaryLinkFresh) return BOAT_POLICY_REJECTED_SAFETY;
            state.mode=MODE_MANUAL;state.missionActive=false;return BOAT_POLICY_ACCEPTED;
        }
        if(cmd.mode==MODE_AUTOPILOT) {
            if(backup || !in.primaryLinkFresh || !navReady(in) || !in.missionTargetValid)
                return BOAT_POLICY_REJECTED_SAFETY;
            state.mode=MODE_AUTOPILOT;state.missionActive=true;return BOAT_POLICY_ACCEPTED;
        }
        if(cmd.mode==MODE_RETURN_HOME) {
            if((backup?!in.backupLinkFresh:!in.primaryLinkFresh) || !navReady(in))
                return BOAT_POLICY_REJECTED_SAFETY;
            state.mode=MODE_RETURN_HOME;state.missionActive=true;out.requestReturnHome=true;
            return BOAT_POLICY_ACCEPTED;
        }
        return BOAT_POLICY_INVALID;
    }
    if(cmd.type==CMD_MANUAL_MOTORS) {
        if(state.mode!=MODE_MANUAL || backup || !in.primaryLinkFresh) return BOAT_POLICY_REJECTED_SAFETY;
        const int16_t max=1000;
        out.leftMotor=limitMotor(cmd.leftMotor,max);
        out.rightMotor=limitMotor(cmd.rightMotor,max);
        return BOAT_POLICY_ACCEPTED;
    }
    int8_t channel=-1;
    switch(cmd.type) {
    case CMD_RELEASE_HOPPER_LEFT: channel=0; break;
    case CMD_RELEASE_HOPPER_RIGHT: channel=1; break;
    case CMD_RELEASE_HOOK_LEFT: channel=2; break;
    case CMD_RELEASE_HOOK_RIGHT: channel=3; break;
    default:
        if(cmd.type==CMD_BAIT_RELEASE || cmd.type==CMD_HOOK_RELEASE) return BOAT_POLICY_INVALID;
        return BOAT_POLICY_INVALID;
    }
    if(!cmd.activate || backup || !in.primaryLinkFresh || !in.motorsNeutral ||
       !in.releaseInterlocksOk || state.mode==MODE_RETURN_HOME ||
       (state.mode==MODE_AUTOPILOT && !in.releaseZoneReached))
        return BOAT_POLICY_REJECTED_SAFETY;
    out.releaseChannel=channel; // request only; physical pulse driver is not implemented
    return BOAT_POLICY_ACCEPTED;
}
