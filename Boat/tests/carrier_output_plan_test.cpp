#include "../source/carrier_output_plan.h"
#include <cassert>
#include <iostream>
int main(){
 CarrierOutputPlan p;CarrierOutputWords o{};
 assert(!p.convert({100,100,-1},0,true,o));
 assert(p.configure(true,false,true,800,30));
 assert(p.convert({500,500,-1},0,true,o)&&o.pwmLeft==400&&o.pwmRight==400&&!o.dirLeft&&o.dirRight);
 assert(p.convert({-500,500,-1},1,true,o)&&o.pwmLeft==0&&o.pwmRight==400);
 assert(p.convert({-500,500,-1},30,true,o)&&o.pwmLeft==0);
 assert(p.convert({-500,500,-1},31,true,o)&&o.pwmLeft==400&&o.dirLeft);
 assert(!p.convert({0,0,2},32,true,o)&&!o.solenoidMask);
 assert(!p.convert({0,0,2},33,true,o));p.reset();
 for(int8_t i=0;i<4;++i){assert(p.convert({0,0,i},34+2*i,true,o)&&o.solenoidMask==(1U<<i)&&!o.pwmLeft&&!o.pwmRight);assert(p.convert({0,0,-1},35+2*i,true,o)&&!o.solenoidMask);}
 assert(p.convert({0,0,0},43,true,o));assert(!p.convert({0,0,1},44,true,o));p.reset();
 assert(p.convert({0,0,0},45,true,o));assert(p.convert({0,0,-1},46,true,o));
 assert(p.convert({200,200,-1},47,true,o)&&!o.pwmLeft);
 assert(p.convert({200,200,-1},76,true,o)&&o.pwmLeft==160);
 assert(!p.convert({200,0,0},77,true,o)&&!o.pwmLeft&&!o.solenoidMask);
 assert(!p.convert({200,200,-1},78,false,o)&&!o.pwmLeft);
 std::cout<<"Carrier normalized PWM, reversal guard and one-hot release passed\n";
}
