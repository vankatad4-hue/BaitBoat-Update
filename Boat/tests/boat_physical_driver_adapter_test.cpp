#include <cassert>
#include "../source/boat_physical_driver_adapter.h"
static uint32_t nowMs=100;
static bool stop=true,zero=true,failWrite=false;
static int8_t coilOut=-1;
static int16_t motorL=0,motorR=0;
static unsigned cuts=0;
static bool sample(BoatHardwareSample &s,BoatSafetyContacts &c,Ekm014Feedback &f){
 s={};c={stop,true,true,true,zero,true,true,true,nowMs};
 f={true,true,zero,true,nowMs};return true;
}
static bool motors(int16_t l,int16_t r,uint16_t){motorL=l;motorR=r;return !failWrite;}
static bool coil(int8_t ch){coilOut=ch;return true;}
static void cut(){++cuts;coilOut=-1;motorL=motorR=0;}
int main(){
 BoatPhysicalDriverAdapter a;
 BoatPhysicalHal h{sample,motors,coil,cut};
 uint16_t widths[4]={100,100,100,100};
 assert(!a.begin(h,widths,false));
 assert(a.begin(h,widths,true));
 BoatHardwareSample s{};
 assert(a.read(s,nowMs));
 assert(a.apply({150,150,-1},100,nowMs));
 assert(motorL==150&&motorR==150&&coilOut==-1);
 nowMs=110;assert(a.read(s,nowMs));
 assert(a.apply({0,0,0},100,nowMs));assert(coilOut==0);
 nowMs=120;zero=false;assert(a.read(s,nowMs));
 assert(a.apply({0,0,0},100,nowMs));
 nowMs=211;zero=true;assert(a.read(s,nowMs));
 assert(!a.apply({0,0,0},100,nowMs)); // expired pulse -> outputs cut
 assert(a.faulted()&&coilOut==-1&&cuts>=2);
 BoatPhysicalDriverAdapter b;assert(b.begin(h,widths,true));
 nowMs=300;stop=false;assert(!b.read(s,nowMs));
 assert(b.faulted());
}
