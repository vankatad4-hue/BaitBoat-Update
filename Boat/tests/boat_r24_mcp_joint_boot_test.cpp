#include "../source/mcp23017_joint_boot_audit_r24.h"
#include <cassert>
#include <cstdint>
#include <iostream>

// Register-only software model. Does NOT represent real EKM014 input
// leakage, PWM pins, solenoid current, motor cutoff, or load acceptance.
struct SimulatedMcp : Mcp23017RegisterBus {
  uint8_t iocon=0x00, dira=0xFF, dirb=0xFF;
  uint8_t olata=0x00, olatb=0x00;
  uint8_t forcedA=0x00, forcedB=0x00;
  bool nackWrites=false, nackReads=false;

  void por() {
    iocon=0; dira=dirb=0xFF; olata=olatb=0;
    forcedA=forcedB=0;
  }
  bool writeRegister(uint8_t a,uint8_t r,uint8_t v) override {
    if (a!=0x27 || nackWrites) return false;
    switch(r) {
      case 0x0A: iocon=v; return true;
      case 0x00: dira=v; return true;
      case 0x01: dirb=v; return true;
      case 0x14: olata=v; return true;
      case 0x15: olatb=v; return true;
      default: return false;
    }
  }
  bool readRegister(uint8_t a,uint8_t r,uint8_t &v) override {
    if (a!=0x27 || nackReads) return false;
    switch(r) {
      case 0x0A: v=iocon; return true;
      case 0x00: v=dira; return true;
      case 0x01: v=dirb; return true;
      case 0x14: v=olata; return true;
      case 0x15: v=olatb; return true;
      case 0x12: v=uint8_t((olata & ~dira) | forcedA); return true;
      case 0x13: v=uint8_t((olatb & ~dirb) | forcedB); return true;
      default: return false;
    }
  }
};
int main() {
  SimulatedMcp bus;
  Mcp23017DirectionAdapter motors(bus);
  Ekm014Mcp23017Adapter releases(bus);
  // POR has all MCP ports as high-impedance inputs; do not call that
  // a commissioned, independently safe actuator interface.
  assert(!boat_r24::registerOutputsDisabled(bus));
  assert(!boat_r24::registerOutputsDisabled(bus,0x20));
  assert(boat_r24::prepareDiagnosticDisabled(bus,motors,releases));
  assert(motors.ready() && releases.ready());
  assert(bus.dira==0xF0 && bus.dirb==0xFC);
  assert(bus.olata==0 && bus.olatb==0);
  assert(boat_r24::registerOutputsDisabled(bus));

  // Any commanded release or direction means outputs are not disabled.
  assert(releases.setChannel(flytec_ekm014_r16::LH_BAIT));
  assert(!boat_r24::registerOutputsDisabled(bus));
  assert(releases.allOff());
  assert(boat_r24::registerOutputsDisabled(bus));
  assert(motors.setDirections(0x02,true,true));
  assert(!boat_r24::registerOutputsDisabled(bus));
  assert(motors.setDirections(0,true,true));
  assert(boat_r24::registerOutputsDisabled(bus));

  // A reset, unexpected direction configuration or port fault invalidates
  // diagnostic readiness even if the I2C address still acknowledges.
  bus.por();
  assert(!boat_r24::registerOutputsDisabled(bus));
  assert(boat_r24::prepareDiagnosticDisabled(bus,motors,releases));
  bus.dirb=0xFF;
  assert(!boat_r24::registerOutputsDisabled(bus));
  bus.dirb=0xFC;
  bus.forcedA=0x01;
  assert(!boat_r24::registerOutputsDisabled(bus));
  bus.forcedA=0;
  bus.olata=0x01;
  assert(!boat_r24::registerOutputsDisabled(bus));
  bus.olata=0;
  bus.iocon=0x80;
  assert(!boat_r24::registerOutputsDisabled(bus));
  bus.iocon=0;
  assert(boat_r24::registerOutputsDisabled(bus));

  // NACK/read failure and failed setup must never produce a "ready" result.
  bus.nackReads=true;
  assert(!boat_r24::registerOutputsDisabled(bus));
  bus.nackReads=false;
  bus.nackWrites=true;
  assert(!boat_r24::prepareDiagnosticDisabled(bus,motors,releases));
  assert(!motors.ready() && !releases.ready());
  bus.nackWrites=false;
  assert(boat_r24::prepareDiagnosticDisabled(bus,motors,releases));
  assert(boat_r24::registerOutputsDisabled(bus));
  std::cout<<"R24 joint MCP23017 startup audit: register-only tests passed; no hardware acceptance\n";
}
