#include <cassert>
#include <cstdint>
#include "../source/mcp23017_direction_adapter.h"
struct FakeBus : Mcp23017RegisterBus {
 uint8_t gpio=0,iodir=0xff,iocon=0xff,latch=0;
 bool failWrite=false,failRead=false,stuck=false;
 int writes=0;
 bool writeRegister(uint8_t addr,uint8_t reg,uint8_t value) override {
   assert(addr==0x20); ++writes;
   if(failWrite)return false;
   if(reg==0x0A)iocon=value;
   if(reg==0x14){latch=value;if(!stuck)gpio=value;}
   if(reg==0x00)iodir=value;
   return true;
 }
 bool readRegister(uint8_t addr,uint8_t reg,uint8_t &value) override {
   assert(addr==0x20);
   if(failRead||reg!=0x12)return false;
   value=gpio;return true;
 }
};
int main(){
 FakeBus b;Mcp23017DirectionAdapter a(b);
 assert(!a.ready());
 assert(!a.setDirections(1,true,true));
 assert(a.prepareDisabled());
 assert(b.iocon==0&&b.iodir==0xfc&&b.latch==0);
 assert(!a.setDirections(1,false,true));
 assert(!a.ready()&&b.latch==0);
 assert(a.prepareDisabled());
 assert(a.setDirections(3,true,true)&&b.gpio==3);
 assert(!a.setDirections(4,true,true));
 assert(!a.ready()&&b.latch==0);
 assert(a.prepareDisabled());
 b.stuck=true;
 assert(!a.setDirections(1,true,true));
 assert(!a.ready()&&b.latch==0);
 b.stuck=false;
 assert(a.prepareDisabled());
 b.failRead=true;
 assert(!a.setDirections(2,true,true));
 assert(!a.ready());
 b.failRead=false;
 b.failWrite=true;
 assert(!a.prepareDisabled());
 assert(!a.ready());
}
