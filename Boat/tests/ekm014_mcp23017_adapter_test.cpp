#include <cassert>
#include <cstdint>
#include "../source/ekm014_mcp23017_adapter.h"
struct FakeBus : Mcp23017RegisterBus {
 uint8_t gpioA=0,dirA=0xff,latchA=0,iocon=0xff;bool failWrite=false,failRead=false,stuck=false;
 bool writeRegister(uint8_t a,uint8_t r,uint8_t v)override{
  assert(a==0x27);if(failWrite)return false;
  if(r==0x0A)iocon=v;
  if(r==0x14){latchA=v;if(!stuck)gpioA=v;}
  if(r==0x00)dirA=v;
  return true;
 }
 bool readRegister(uint8_t a,uint8_t r,uint8_t &v)override{
  assert(a==0x27);if(failRead||r!=0x12)return false;v=gpioA;return true;
 }
};
int main(){
 FakeBus bus;Ekm014Mcp23017Adapter io(bus);
 assert(!io.ready());assert(io.beginDisabled());
 assert(bus.iocon==0&&bus.latchA==0&&bus.dirA==0xF0&&io.activeMask()==0);
 assert(io.setChannel(flytec_ekm014_r16::LH_BAIT)&&io.activeMask()==1);
 assert(!io.setChannel(flytec_ekm014_r16::RH_BAIT)&&io.activeMask()==1);
 assert(io.allOff()&&io.activeMask()==0);
 assert(io.setChannel(flytec_ekm014_r16::RK_HOOK)&&io.activeMask()==8);
 assert(io.allOff());
 bus.stuck=true;
 assert(!io.setChannel(flytec_ekm014_r16::LK_HOOK));
 assert(!io.ready());
 bus.stuck=false;bus.failRead=true;
 assert(!io.beginDisabled()&&!io.ready());
 bus.failRead=false;bus.failWrite=true;
 assert(!io.beginDisabled()&&!io.ready());
}
