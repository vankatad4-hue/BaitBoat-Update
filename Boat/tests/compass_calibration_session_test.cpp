#include <cassert>
#include <cmath>
#include "../source/compass_calibration_session.h"
int main(){
 CompassCalibrationSession s;CompassCalibrationCandidate out{};
 assert(!s.finish(out));s.start();
 for(int i=0;i<360;++i){
  double a=i*3.141592653589793/180.0;
  // Three orthogonal sweeps with realistic offsets and axis distortion.
  assert(s.add({12+35*cos(a),-8+30*sin(a),5}));
  assert(s.add({12+35*cos(a),-8,5+28*sin(a)}));
  assert(s.add({12,-8+30*cos(a),5+28*sin(a)}));
 }
 assert(s.finish(out));assert(out.samples==1080);
 assert(std::fabs(out.offsetUt[0]-12)<0.1);
 assert(std::fabs(out.offsetUt[1]+8)<0.1);
 assert(std::fabs(out.offsetUt[2]-5)<0.1);
 assert(out.scale[0]>0.5&&out.scale[0]<2);
 s.start();for(int i=0;i<200;++i)assert(s.add({20,25,30}));
 assert(!s.finish(out));
 s.start();assert(!s.add({NAN,2,3}));assert(!s.finish(out));
}
