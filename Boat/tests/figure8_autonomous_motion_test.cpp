#include <cassert>
#include "../source/figure8_autonomous_motion.h"
int main(){
 Figure8AutonomousMotion a;
 Figure8MotionSafety s{};
 assert(!a.start(1000,s));
 s={true,true,true,true,true,true,true,true,true,true,false};
 assert(a.start(1000,s));
 auto v=a.tick(1100,s,Figure8AutoState::COLLECTING);
 assert(v.valid&&v.left==180&&v.right==100);
 v=a.tick(13100,s,Figure8AutoState::COLLECTING);
 // Missing 250ms control-loop lease must stop even if all other flags good.
 assert(!v.valid&&a.state()==Figure8MotionState::FAILED);
 Figure8AutonomousMotion b;assert(b.start(0,s));
 v=b.tick(100,s,Figure8AutoState::COLLECTING);assert(v.valid);
 s.manualOverride=true;
 v=b.tick(150,s,Figure8AutoState::COLLECTING);
 assert(!v.valid&&b.state()==Figure8MotionState::FAILED);
 s.manualOverride=false;
 Figure8AutonomousMotion c;assert(c.start(0,s));
 v=c.tick(100,s,Figure8AutoState::CANDIDATE_READY);
 assert(!v.valid&&c.state()==Figure8MotionState::COMPLETE);
 Figure8AutonomousMotion d;assert(d.start(0,s));d.abort();
 assert(!d.tick(100,s,Figure8AutoState::COLLECTING).valid);
 Figure8AutonomousMotion e;assert(e.start(0,s));
 s.physicalStopReleased=false;
 assert(!e.tick(10,s,Figure8AutoState::COLLECTING).valid);
}
