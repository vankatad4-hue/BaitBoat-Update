#include <cassert>
#include <cmath>
#include "../source/figure8_compass_calibration.h"
#include "../source/m100_compass_qmc5883l.h"
int main(){
 Figure8CompassCalibration s;Figure8Candidate c{};
 assert(!s.finish(c));s.start();
 for(int i=0;i<720;++i){
  const double a=i*3.141592653589793/180.0;
  assert(s.add({5+35*cos(a),-3+30*sin(a),12}));
 }
 assert(s.finish(c));assert(c.samples==720);assert(c.occupiedSectors>=9);
 assert(std::fabs(c.offsetX-5)<0.2&&std::fabs(c.offsetY+3)<0.2);
 s.start();for(int i=0;i<250;++i)assert(s.add({30,10,5}));
 assert(!s.finish(c));
 s.start();assert(!s.add({NAN,1,2}));assert(!s.finish(c));
 uint8_t q[7]={0x84,0x03,0,0,0,0,1};SensorVector v{};
 assert(m100Qmc5883lIdentity(0xff));assert(!m100Qmc5883lIdentity(0));
 assert(m100Qmc5883lDecode(q,v));assert(std::fabs(v.x-30)<0.001);
 q[6]=0;assert(!m100Qmc5883lDecode(q,v));
}
