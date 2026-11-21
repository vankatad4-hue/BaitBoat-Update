#include <cassert>
#include "../source/figure8_closed_loop.h"
int main(){
 Figure8MotionSafety s{true,true,true,true,true,true,true,true,true,true,false};
 Figure8NavSample n{0,true,0,true};
 Figure8ClosedLoop c;
 assert(c.start(0,s,n));
 n.speedMps=0.5;n.headingDeg=3;
 auto out=c.tick(100,s,n,Figure8AutoState::COLLECTING);
 assert(out.valid&&out.left>out.right&&c.leg()==0);
 n.headingDeg=350;
 assert(c.tick(200,s,n,Figure8AutoState::COLLECTING).valid);
 n.headingFresh=false;
 assert(!c.tick(250,s,n,Figure8AutoState::COLLECTING).valid);
 Figure8ClosedLoop d;n.headingFresh=true;n.headingDeg=0;n.speedMps=0;
 assert(d.start(0,s,n));n.speedMps=0.5;
 assert(!d.tick(300,s,n,Figure8AutoState::COLLECTING).valid);
}
