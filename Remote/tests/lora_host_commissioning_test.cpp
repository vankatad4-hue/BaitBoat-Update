#include "lora_link_policy.h"
#include "sc16is750_fifo_budget.h"
#include <cassert>
using namespace flytec_rtos;
struct Registers {
 uint8_t lcr=3,mcr=0,ier=0,lsr=0x60,tx=64,rx=0,dll=12,dlh=0;
 bool wrongLatch=false,wrongDivisor=false;unsigned calls=0,failAt=0,divisorWrites=0;
 bool allow(){return ++calls!=failAt;}
 bool read(uint8_t r,uint8_t& v){
  if(!allow())return false;
  switch(r){case 0:v=uint8_t(dll^(wrongDivisor?1U:0U));return true;
   case 1:v=lcr&0x80?dlh:ier;return true;case 3:v=lcr;return true;
   case 4:v=mcr;return true;case 5:v=lsr;return true;case 8:v=tx;return true;case 9:v=rx;return true;default:return false;}
 }
 bool write(uint8_t r,uint8_t v){
  if(!allow())return false;
  if(r==3){if(!(wrongLatch&&v==0x83))lcr=v;return true;}
  // Aliased THR/IER registers must never be touched with DLAB clear.
  assert(lcr==0x83);++divisorWrites;if(r==0)dll=v;else if(r==1)dlh=v;else assert(false);return true;
 }
 bool configure(uint32_t crystal=1843200,uint32_t baud=115200){
  return sc16CommissioningSetBaud([&](uint8_t r,uint8_t& v){return read(r,v);},[&](uint8_t r,uint8_t v){return write(r,v);},crystal,baud);
 }
};
int main(){
 static_assert(r30h_sc16::E22_GPIO_OUTPUT_MASK==0x0B,"M0/M1/reset outputs; AUX input");
 assert(uartBaudWithinOnePercent(115201,115200)&&uartBaudWithinOnePercent(9599,9600));
 assert(!uartBaudWithinOnePercent(9600,115200)&&!uartBaudWithinOnePercent(19200,19200));
 Registers ok;assert(ok.configure()&&ok.dll==1&&ok.dlh==0&&ok.lcr==3&&ok.divisorWrites==2);const auto steps=ok.calls;
 assert(ok.configure(1843200,9600)&&ok.dll==12&&ok.lcr==3);
 for(unsigned fail=1;fail<=steps;++fail){Registers r;r.failAt=fail;assert(!r.configure());}
 Registers shifting;shifting.lsr=0x20;assert(!shifting.configure()&&!shifting.divisorWrites);
 Registers full;full.tx=63;assert(!full.configure()&&!full.divisorWrites);
 Registers received;received.rx=1;assert(!received.configure()&&!received.divisorWrites);
 Registers error;error.lsr=0x62;assert(!error.configure()&&!error.divisorWrites);
 Registers prescale;prescale.mcr=0x80;assert(!prescale.configure()&&!prescale.divisorWrites);
 Registers sleep;sleep.ier=0x10;assert(!sleep.configure()&&!sleep.divisorWrites);
 Registers badClock;assert(!badClock.configure(1000000,115200)&&!badClock.calls);
 Registers noBank;noBank.wrongLatch=true;assert(!noBank.configure()&&!noBank.divisorWrites&&noBank.lcr==3);
 Registers wrong;wrong.wrongDivisor=true;assert(!wrong.configure()&&wrong.divisorWrites==2&&wrong.lcr==3);
 LoRaAirtimeProfile profile;profile.commissioned=true;LoRaAirtimePolicy policy;LoRaQuietRestart quiet;
 const uint32_t start=0xffff0000U;quiet.observe(start,true);quiet.observe(start+3600000U,true);
 assert(!quiet.ready(start+3600000U)&&!policy.configureAfterQuiet(profile,start+3600000U,quiet));
 quiet.observe(start,false);for(uint32_t age=0;age<=3600000U;age+=1000U)quiet.observe(start+age,true);
 assert(quiet.ready(start+3600000U)&&!quiet.ready(start+3601001U));
 assert(policy.configureAfterQuiet(profile,start+3600000U,quiet));
 LoRaAirtimePolicy second;assert(!quiet.ready(start+3600000U)&&!second.configureAfterQuiet(profile,start+3600000U,quiet));
 assert(policy.reserve(LoRaAirtimePolicy::Traffic::Emergency,start+3600000U,true)==LoRaAirtimePolicy::Decision::Allowed);
 quiet.observe(start+3600001U,false);assert(!quiet.ready(start+3600001U));
}
