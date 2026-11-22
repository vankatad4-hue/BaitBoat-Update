#include "../source/boat_hardware_supervisor.h"
#include <cassert>
#include <iostream>
static BoatHardwareSample sample(uint32_t t){
 return {{0,1000,1,90,true,true,true,true},t,t,t,true,true,true,true,true,true,true,true,false,false,75,3,8,12000,0,0,0,true,true,BOAT_BATTERY_GREEN,false};
}
int main(){
 BoatActuatorSupervisor s;auto hw=sample(0);
 assert(!s.configure({{0,0,0,0},0,0}));
 assert(!s.clearStop(hw,0,true,true));
 assert(s.configure({{100,150,200,250},400,3}));
 assert(!s.clearStop(hw,100,true,true));
 assert(!s.clearStop(hw,0,false,true));
 assert(s.clearStop(hw,0,true,true));
 ExecInputs in=boatHardware_inputs(hw,0,s.stopped());in.primaryFresh=true;
 const ExecOutput stop={INTENT_STOP,-1,0};const MissionNavigationResult nav{};
 s.manual(200,-200,0,true);
 assert(s.tick(in,199,stop,nav,false,false,0,0,199).left==200);
 assert(s.tick(in,200,stop,nav,false,false,0,0,200).left==0);
 assert(s.beginRelease(1,1,0,in,300));assert(!s.beginRelease(1,2,1,in,301));
 assert(s.tick(in,399,stop,nav,true,false,0,0,399).releaseChannel==0);
 assert(s.tick(in,400,stop,nav,true,false,0,0,400).releaseChannel==-1);
 assert(!s.takeCompletion().valid);assert(s.tick(in,401,stop,nav,true,false,0,0,400).releaseChannel==-1);assert(!s.takeCompletion().valid);assert(s.tick(in,401,stop,nav,true,false,0,0,401).releaseChannel==-1);
 auto done=s.takeCompletion();assert(done.valid&&done.success&&done.token==1);
 assert(!s.beginRelease(1,1,0,in,402));
 assert(s.beginRelease(1,2,1,in,403));in.primaryFresh=false;
 assert(s.tick(in,404,stop,nav,true,false,0,0,404).releaseChannel==-1);
 done=s.takeCompletion();assert(done.valid&&!done.success);
 assert(s.stopped());hw=sample(404);assert(s.clearStop(hw,404,true,true));
 in.primaryFresh=true;in.backupFresh=false; // restore the simulated primary radio
 MissionNavigationResult home={true,false,15,90,0};
 assert(s.tick(in,405,{INTENT_NAV_HOME,-1,0},home,true,false,0,0,405).left==400);
 // Autopilot turns with unequal forward thrust, without any rudder or
 // unapproved negative motor demand / counter-rotating pivot.
 auto rightTurn=home;rightTurn.headingErrorDeg=85;
 const auto rt=s.tick(in,405,{INTENT_NAV_HOME,-1,0},rightTurn,true,false,0,0,405);
 assert(rt.left>rt.right&&rt.left>=0&&rt.right>=0&&rt.left<=400&&rt.right<=400&&rt.releaseChannel==-1);
 auto leftTurn=home;leftTurn.headingErrorDeg=-85;
 const auto lt=s.tick(in,405,{INTENT_NAV_HOME,-1,0},leftTurn,true,false,0,0,405);
 assert(lt.right>lt.left&&lt.left>=0&&lt.right>=0&&lt.left<=400&&lt.right<=400);
 for(double error : {-179.0,-90.0,-45.0,0.0,45.0,90.0,179.0}){
  auto n=home;n.headingErrorDeg=error;
  const auto drive=s.tick(in,405,{INTENT_NAV_HOME,-1,0},n,true,false,0,0,405);
  assert(drive.left>=0&&drive.right>=0&&drive.left<=400&&drive.right<=400);
  if(error>0)assert(drive.left>=drive.right);
  if(error<0)assert(drive.right>=drive.left);
 }
 auto arrived=home;arrived.arrived=true;
 const auto halted=s.tick(in,405,{INTENT_NAV_HOME,-1,0},arrived,true,false,0,0,405);
 assert(halted.left==0&&halted.right==0&&halted.releaseChannel==-1);
 in.stopActive=true;assert(s.tick(in,406,{INTENT_NAV_HOME,-1,0},home,true,false,0,0,406).left==0&&s.stopped());
 hw=sample(500);assert(s.clearStop(hw,500,true,true));
 assert(boatHardware_inputs(hw,600,false).stopActive);
 auto fresh=boatHardware_inputs(sample(700),700,false);assert(!fresh.stopActive);

 // Radio watchdog: total loss returns all outputs OFF without latching a
 // normal propulsion stop; a restored link does not replay stale sticks.
 BoatActuatorSupervisor radio;
 auto radioHw=sample(700);
 assert(radio.configure({{100,150,200,250},400,3}));
 assert(radio.clearStop(radioHw,700,true,true));
 ExecInputs rin=boatHardware_inputs(radioHw,700,false);
 rin.primaryFresh=true;rin.backupFresh=false;
 radio.manual(300,300,700,true);
 assert(radio.tick(rin,701,stop,nav,false,false,0,0,701).left==300);
 rin.primaryFresh=false;rin.backupFresh=false;
 auto radioOff=radio.tick(rin,702,stop,nav,false,false,0,0,702);
 assert(radioOff.left==0&&radioOff.right==0&&radioOff.releaseChannel==-1);
 assert(!radio.stopped());
 rin.primaryFresh=true;
 radio.manual(300,300,703,true); // stale non-neutral stick is rejected
 assert(radio.tick(rin,704,stop,nav,false,false,0,0,704).left==0);
 radio.manual(0,0,705,true);     // explicit neutral re-arms manual input
 radio.manual(300,300,706,true);
 assert(radio.tick(rin,707,stop,nav,false,false,0,0,707).left==300);

 // Backup-only link can sustain autonomous navigation, but not manual sticks.
 rin.primaryFresh=false;rin.backupFresh=true;
 MissionNavigationResult radioHome={true,false,15,90,0};
 const auto backupDrive=radio.tick(rin,708,{INTENT_NAV_HOME,-1,0},radioHome,true,false,0,0,708);
 assert(backupDrive.left>0&&backupDrive.right>0);
 rin.primaryFresh=false;rin.backupFresh=false;
 const auto noLink=radio.tick(rin,709,{INTENT_NAV_HOME,-1,0},radioHome,true,false,0,0,709);
 assert(noLink.left==0&&noLink.right==0&&noLink.releaseChannel==-1);
 rin.backupFresh=true;
 const auto recovered=radio.tick(rin,710,{INTENT_NAV_HOME,-1,0},radioHome,true,false,0,0,710);
 assert(recovered.left>0&&recovered.right>0);
 rin.primaryFresh=false;rin.backupFresh=false;rin.autonomousRth=true;
 const auto detachedHome=radio.tick(rin,711,{INTENT_NAV_HOME,-1,0},radioHome,true,false,0,0,711);
 assert(detachedHome.left>0&&detachedHome.right>0&&detachedHome.releaseChannel==-1);
 const auto detachedTarget=radio.tick(rin,712,{INTENT_NAV_TARGET,-1,0},radioHome,true,false,0,0,712);
 assert(detachedTarget.left==0&&detachedTarget.right==0&&detachedTarget.releaseChannel==-1);
 rin.autonomousRth=false;
 const auto stillNoLink=radio.tick(rin,713,{INTENT_NAV_HOME,-1,0},radioHome,true,false,0,0,713);
 assert(stillNoLink.left==0&&stillNoLink.right==0&&stillNoLink.releaseChannel==-1);

 // R79: low battery interrupts an already-active release. A physical
 // post-off idle/current-zero sample must be newer than the deenergizing tick.
 BoatActuatorSupervisor low;
 auto lowHw=sample(1000);
 assert(low.configure({{100,150,200,250},400,3}));
 assert(low.clearStop(lowHw,1000,true,true));
 auto lin=boatHardware_inputs(lowHw,1000,false);lin.primaryFresh=true;
 assert(low.beginRelease(3,1,0,lin,1001));
 assert(low.tick(lin,1002,stop,nav,true,false,0,0,1002).releaseChannel==0);
 lin.batteryLow=true;
 assert(low.tick(lin,1003,stop,nav,true,false,0,0,1003).releaseChannel==-1);
 assert(low.releaseBusy());
 assert(low.tick(lin,1004,stop,nav,true,false,0,0,1003).releaseChannel==-1);
 assert(low.releaseBusy()); // same safety sample cannot prove deenergization
 assert(low.tick(lin,1005,stop,nav,true,false,0,0,1005).releaseChannel==-1);
 assert(!low.releaseBusy());
 auto cancelled=low.takeCompletion();assert(cancelled.valid&&!cancelled.success);
 assert(!low.stopped());
 assert(low.tick(lin,1006,{INTENT_NAV_HOME,-1,0},home,true,false,0,0,1006).left>0);

 BoatActuatorSupervisor badFeedback;
 assert(badFeedback.configure({{100,150,200,250},400,3}));
 assert(badFeedback.clearStop(lowHw,1000,true,true));
 auto bin=boatHardware_inputs(lowHw,1000,false);bin.primaryFresh=true;
 assert(badFeedback.beginRelease(4,1,0,bin,1001));
 bin.batteryLow=true;bin.releaseIdle=false;
 assert(badFeedback.tick(bin,1002,stop,nav,true,false,0,0,1002).releaseChannel==-1);
 assert(badFeedback.releaseBusy());
 assert(badFeedback.tick(bin,1503,stop,nav,true,false,0,0,1503).releaseChannel==-1);
 assert(badFeedback.stopped()); // missing physical zero feedback: STOP, not RTH

 std::cout<<"Hardware sample validity, radio watchdog, battery release abort and release executor passed\n";
}
