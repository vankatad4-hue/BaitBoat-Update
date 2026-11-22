#include "../source/boat_command_policy.h"
#include <assert.h>

static BoatCommand command(BoatCommandType type) {
  BoatCommand c={}; c.type=type; c.activate=true; c.mode=MODE_MANUAL; return c;
}
static BoatPolicyInputs ready() {
  BoatPolicyInputs i={}; i.primaryLinkFresh=true; i.backupLinkFresh=true;
  i.physicalStopReleased=true; i.batteryHealthy=true; i.motorsNeutral=true;
  i.gnssFresh=true; i.headingFresh=true; i.geofenceValid=true; i.homeValid=true;
  i.missionTargetValid=true; i.releaseZoneReached=true; i.releaseInterlocksOk=true;
  return i;
}
int main() {
  BoatPolicyState s; boatPolicyInit(s); BoatPolicyOutput o={};
  BoatPolicyInputs i=ready();
  assert(s.stopLatched && s.mode==MODE_MANUAL && !s.missionActive);
  BoatCommand c=command(CMD_MANUAL_MOTORS); c.leftMotor=800;c.rightMotor=-800;
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_REJECTED_SAFETY);
  assert(o.leftMotor==0 && o.rightMotor==0 && o.releaseChannel==-1);

  c=command(CMD_EMERGENCY_STOP); i.primaryLinkFresh=false;i.backupLinkFresh=false;
  assert(boatPolicyApply(c,true,i,s,o)==BOAT_POLICY_ACCEPTED);
  assert(s.stopLatched && !s.missionActive);
  c=command(CMD_CLEAR_STOP);
  assert(boatPolicyApply(c,true,i,s,o)==BOAT_POLICY_REJECTED_SAFETY);
  i=ready();i.motorsNeutral=false;
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_REJECTED_SAFETY);
  i=ready();
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_ACCEPTED && !s.stopLatched);

  c=command(CMD_MANUAL_MOTORS);c.leftMotor=1200;c.rightMotor=-1200;
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_ACCEPTED);
  assert(o.leftMotor==1000 && o.rightMotor==-1000);
  i.primaryLinkFresh=false;i.backupLinkFresh=true;
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_REJECTED_SAFETY); // primary frame requires primary freshness
  assert(boatPolicyApply(c,true,i,s,o)==BOAT_POLICY_REJECTED_SAFETY);
  assert(o.leftMotor==0 && o.rightMotor==0);
  i=ready();

  c=command(CMD_SET_MODE);c.mode=MODE_AUTOPILOT;
  i.gnssFresh=false;
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_REJECTED_SAFETY);
  i=ready();
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_ACCEPTED);
  assert(s.mode==MODE_AUTOPILOT && s.missionActive);
  c=command(CMD_RELEASE_HOOK_RIGHT);i.releaseZoneReached=false;
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_REJECTED_SAFETY && o.releaseChannel==-1);
  i.releaseZoneReached=true;
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_ACCEPTED && o.releaseChannel==3);
  assert(boatPolicyApply(c,true,i,s,o)==BOAT_POLICY_REJECTED_SAFETY && o.releaseChannel==-1);

  c=command(CMD_SET_MODE);c.mode=MODE_RETURN_HOME;
  i.primaryLinkFresh=false;i.backupLinkFresh=true;
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_REJECTED_SAFETY); // link-specific freshness
  assert(boatPolicyApply(c,true,i,s,o)==BOAT_POLICY_ACCEPTED && o.requestReturnHome);
  assert(s.mode==MODE_RETURN_HOME);
  c=command(CMD_RELEASE_HOPPER_LEFT);
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_REJECTED_SAFETY);

  c=command(CMD_BAIT_RELEASE);
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_INVALID);
  c=command(CMD_EMERGENCY_STOP);
  assert(boatPolicyApply(c,false,i,s,o)==BOAT_POLICY_ACCEPTED && s.stopLatched);
  return 0;
}
