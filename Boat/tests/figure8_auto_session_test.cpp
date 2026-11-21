#include <cassert>
#include <cmath>
#include "../source/figure8_auto_session.h"
int main(){
 Figure8AutoSession s;Figure8Candidate g{},m{};
 assert(!s.start(1000,true,false));
 assert(s.start(1000,true,true));
 assert(!s.start(1000,true,true));
 for(int i=0;i<720;++i){
  const double a=i*3.141592653589793/180.;
  const uint32_t now=1000+i*50;
  s.addGy({4+35*cos(a),-3+32*sin(a),8},now);
  s.addM100({-2+38*cos(a),5+29*sin(a),7},now);
  s.update(now);
 }
 assert(s.state()==Figure8AutoState::CANDIDATE_READY);
 assert(s.candidates(g,m));assert(g.samples>=150&&m.samples>=150);
 Figure8AutoSession missing;assert(missing.start(0,true,true));
 for(int i=0;i<150;++i)missing.addGy({30,20,1},i*10);
 missing.update(6000);assert(missing.state()==Figure8AutoState::TIMED_OUT);
 Figure8AutoSession aborted;assert(aborted.start(0,true,true));aborted.abort();
 assert(aborted.state()==Figure8AutoState::ABORTED);
 assert(!aborted.candidates(g,m));
}
