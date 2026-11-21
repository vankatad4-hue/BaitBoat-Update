#include "../source/mdd20a_direct_safety.h"
#include <cassert>
#include <iostream>
static Mdd20aDirectFeedback good(uint32_t now){
 return {now,true,true,true,true,true,true,true,true};
}
int main(){
 Mdd20aDirectSafety s;CarrierOutputWords o{};
 const uint64_t mask=(uint64_t(1)<<6)-1;
 auto p=Mdd20aDirectProfile{true,0,1,2,3,4,5};
 assert(!s.prepare({100,100,-1},good(0),0,o));
 assert(s.configure(p,mask,false,true,800,30));
 assert(s.prepare({400,-400,-1},good(0),0,o));
 assert(o.pwmLeft==320&&o.pwmRight==320);
 // Reconfigure for independent cases: every input failure must latch.
 assert(s.configure(p,mask,false,true,800,30));
 auto fb=good(10);fb.physicalStopReleased=false;
 assert(!s.prepare({400,0,-1},fb,10,o)&&s.failed()&&!o.pwmLeft&&!o.pwmRight);
 assert(!s.prepare({0,0,-1},good(11),11,o));
 assert(s.configure(p,mask,false,true,800,30));
 fb=good(20);fb.sampleAtMs=0;
 assert(!s.prepare({400,0,-1},fb,50,o)&&s.failed());
 assert(s.configure(p,mask,false,true,800,30));
 fb=good(60);fb.gateFeedbackOn=false;
 assert(!s.prepare({400,0,-1},fb,60,o)&&s.failed());
 assert(s.configure(p,mask,false,true,800,30));
 assert(s.prepare({0,0,0},good(70),70,o)&&!o.pwmLeft&&!o.pwmRight);
 assert(s.prepare({0,0,-1},good(71),71,o));
 assert(s.prepare({400,0,-1},good(100),100,o)&&!o.pwmLeft);
 assert(s.prepare({400,0,-1},good(102),102,o)&&!o.pwmLeft);
 assert(s.prepare({400,0,-1},good(131),131,o)&&o.pwmLeft==320);
 assert(s.configure(p,mask,false,true,800,30));
 auto unsafe=good(141);unsafe.motorsIdle=false;
 assert(!s.prepare({0,0,1},unsafe,141,o)&&s.failed());
 assert(s.configure(p,mask,false,true,800,30));
 assert(s.prepare({400,0,-1},good(141),141,o)&&o.pwmLeft==320);
 assert(!s.prepare({0,0,1},good(142),142,o)&&s.failed());
 std::cout<<"Direct MDD20A local safety gate and fault latch passed\n";
}
