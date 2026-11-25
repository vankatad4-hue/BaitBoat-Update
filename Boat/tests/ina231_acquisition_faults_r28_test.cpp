#include "../source/ina231_acquisition.h"
#include "../source/boat_battery_policy.h"
#include <cassert>
#include <iostream>
using namespace boat_r28;
struct FakeBus final : RegisterBus {
 bool nackWrite=false,nackConfig=false,nackMask=false,nackVoltage=false;
 bool misconfigured=false,ready=true,overflow=false,invalidMsb=false;
 uint16_t value=10080, lastConfig=0;
 unsigned starts=0,maskReads=0,busReads=0;
 bool write16(uint8_t addr,uint8_t reg,uint16_t v) override {
  if(addr!=0x40 || reg!=0 || nackWrite)return false;
  starts++;lastConfig=v;return true;
 }
 bool read16(uint8_t addr,uint8_t reg,uint16_t &v) override {
  if(addr!=0x40)return false;
  if(reg==0){if(nackConfig)return false;v=misconfigured?0x4127:lastConfig;return true;}
  if(reg==6){maskReads++;if(nackMask)return false;v=(ready?8U:0U)|(overflow?4U:0U);return true;}
  if(reg==2){busReads++;if(nackVoltage)return false;v=value|(invalidMsb?0x8000U:0U);return true;}
  return false;
 }
};
int main(){
 static_assert((CONFIG_BUS_TRIGGERED&7U)==2,"Only BUS single-shot mode");
 static_assert(!boat_r27::INA231_PHYSICAL_COMMISSIONED,"Boat must stay uncommissioned");
 FakeBus fake; VoltageSampler sensor;
 assert(sensor.poll(fake,0)==PollResult::Disabled);
 assert(fake.starts==0);
 assert(!sensor.begin(false,0x40));
 assert(sensor.poll(fake,0)==PollResult::Disabled);
 assert(!sensor.begin(true,0x48));
 assert(fake.starts==0);
 // Synthetic sign-off only: this is NOT a physical commissioning flag.
 assert(sensor.begin(true,0x40));
 assert(sensor.poll(fake,0)==PollResult::Waiting);
 assert(fake.starts==1 && fake.lastConfig==0x4122);
 assert(sensor.poll(fake,1)==PollResult::Waiting && fake.maskReads==0);
 assert(sensor.poll(fake,2)==PollResult::Fresh);
 assert(sensor.snapshot(2).valid && sensor.snapshot(2).millivolts==12600);
 assert(sensor.poll(fake,198)==PollResult::Waiting && fake.starts==1);
 assert(sensor.poll(fake,200)==PollResult::Waiting && fake.starts==2);
 assert(sensor.snapshot(200).valid && sensor.snapshot(200).acquiredAtMs==2);
 fake.ready=false;
 assert(sensor.poll(fake,203)==PollResult::Waiting);
 assert(sensor.snapshot(203).acquiredAtMs==2);
 assert(sensor.poll(fake,241)==PollResult::Failed);
 assert(!sensor.snapshot(241).valid);
 fake.ready=true;
 assert(sensor.poll(fake,242)==PollResult::Waiting);
 assert(sensor.poll(fake,244)==PollResult::Fresh);
 fake.nackMask=true;
 assert(sensor.poll(fake,442)==PollResult::Waiting);
 assert(sensor.poll(fake,445)==PollResult::Failed);
 assert(!sensor.snapshot(445).valid);
 fake.nackMask=false;fake.nackVoltage=true;
 assert(sensor.poll(fake,446)==PollResult::Waiting);
 assert(sensor.poll(fake,449)==PollResult::Failed);
 fake.nackVoltage=false;fake.misconfigured=true;
 assert(sensor.poll(fake,450)==PollResult::Failed && !sensor.snapshot(450).valid);
 fake.misconfigured=false;fake.nackWrite=true;
 assert(sensor.poll(fake,451)==PollResult::Failed);
 fake.nackWrite=false;fake.nackConfig=true;
 assert(sensor.poll(fake,452)==PollResult::Failed);
 fake.nackConfig=false;fake.overflow=true;
 assert(sensor.poll(fake,453)==PollResult::Waiting);
 assert(sensor.poll(fake,456)==PollResult::Failed);
 fake.overflow=false;fake.invalidMsb=true;
 assert(sensor.poll(fake,457)==PollResult::Waiting);
 assert(sensor.poll(fake,460)==PollResult::Failed);
 fake.invalidMsb=false;fake.value=0;
 assert(sensor.poll(fake,461)==PollResult::Waiting);
 assert(sensor.poll(fake,464)==PollResult::Failed);
 fake.value=7920;
 assert(sensor.poll(fake,465)==PollResult::Waiting);
 assert(sensor.poll(fake,468)==PollResult::Fresh);
 assert(sensor.snapshot(468).millivolts==9900);
 BoatBatteryMonitor monitor;
 auto battery=monitor.update(sensor.snapshot(468).valid,sensor.snapshot(468).millivolts,468);
 assert(battery.critical && battery.band==BOAT_BATTERY_CRITICAL);
 battery=monitor.update(false,0,469);
 assert(!battery.valid && battery.critical);
 assert(!sensor.snapshot(1469).valid);
 // 32-bit millis rollover and sample-age arithmetic.
 sensor.reset();fake.value=8640;
 assert(sensor.begin(true,0x40));
 assert(sensor.poll(fake,0xfffffffeU)==PollResult::Waiting);
 assert(sensor.poll(fake,1)==PollResult::Fresh);
 assert(sensor.snapshot(1).millivolts==10800);
 assert(!sensor.snapshot(1002).valid);
 sensor.reset();
 assert(sensor.poll(fake,2000)==PollResult::Disabled);
 std::cout<<"R28 INA231 triggered I2C fault simulation PASS (no outputs/no hardware)\n";
}
