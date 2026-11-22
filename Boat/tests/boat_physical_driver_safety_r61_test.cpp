#include <cassert>
#include "../source/boat_physical_driver_adapter.h"

static uint32_t clockMs=100;
static uint32_t feedbackAt=0;
static bool contactZero=true,senseZero=true,stopClosed=true;
static int8_t coilOutput=-1;
static int16_t leftOutput=0,rightOutput=0;
static unsigned cutCount=0;
static bool sample(BoatHardwareSample &s,BoatSafetyContacts &c,Ekm014Feedback &f){
 s={};s.voltageValid=true;s.batteryAt=clockMs;s.batteryMillivolts=12000;s.batteryHealthy=true;
 c={stopClosed,true,true,true,contactZero,true,true,true,clockMs};
 f={true,true,senseZero,true,feedbackAt?feedbackAt:clockMs};
 return true;
}
static bool writeMotors(int16_t left,int16_t right,uint16_t){
 leftOutput=left;rightOutput=right;return true;
}
static bool writeCoil(int8_t channel){coilOutput=channel;return true;}
static void cut(){++cutCount;leftOutput=rightOutput=0;coilOutput=-1;}
static BoatPhysicalHal hal(){return {sample,writeMotors,writeCoil,cut};}

int main(){
 const uint16_t widths[4]={100,100,100,100};
 BoatHardwareSample s{};
 // A pulse must finish and current must be observed zero after turnoff.
 BoatPhysicalDriverAdapter safe;
 assert(safe.begin(hal(),widths,true));
 clockMs=100;assert(safe.read(s,clockMs));
 assert(safe.apply({0,0,1},100,clockMs));assert(coilOutput==1);
 clockMs=125;contactZero=senseZero=false;
 assert(safe.read(s,clockMs));
 assert(safe.apply({0,0,1},100,clockMs));
 clockMs=201;assert(safe.read(s,clockMs));
 assert(safe.apply({0,0,-1},100,clockMs));assert(coilOutput==-1);
 clockMs=202;contactZero=senseZero=true;assert(safe.read(s,clockMs));
 assert(safe.apply({120,120,-1},100,clockMs));
 assert(leftOutput==120 && !safe.faulted());
 // A mismatch between contact and independent current feedback is a fault.
 BoatPhysicalDriverAdapter mismatch;
 assert(mismatch.begin(hal(),widths,true));
 clockMs=210;contactZero=true;senseZero=false;
 assert(!mismatch.read(s,clockMs));assert(mismatch.faulted());
 assert(leftOutput==0 && rightOutput==0 && coilOutput==-1);
 // Stale independent feedback must not be interpreted as cleared pulse.
 BoatPhysicalDriverAdapter stale;
 contactZero=senseZero=true;clockMs=300;
 assert(stale.begin(hal(),widths,true));
 assert(stale.read(s,clockMs));assert(stale.apply({0,0,0},100,clockMs));
 clockMs=320;contactZero=senseZero=false;
 assert(stale.read(s,clockMs));assert(stale.apply({0,0,0},100,clockMs));
 clockMs=405;assert(stale.read(s,clockMs));
 assert(stale.apply({0,0,-1},100,clockMs)); // await current zero
 clockMs=406;contactZero=senseZero=true;feedbackAt=320;
 assert(stale.read(s,clockMs));
 assert(!stale.apply({200,200,-1},100,clockMs));
 assert(stale.faulted()&&leftOutput==0&&rightOutput==0);
 // STOP contact rejection, without a motor command, remains hard latched.
 BoatPhysicalDriverAdapter stopped;
 feedbackAt=0;stopClosed=false;clockMs=500;
 assert(stopped.begin(hal(),widths,true));
 assert(!stopped.read(s,clockMs));
 assert(stopped.faulted() && cutCount>=4);
}
