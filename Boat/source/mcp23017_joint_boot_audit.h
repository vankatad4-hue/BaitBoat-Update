#pragma once
#include <stdint.h>
#include "mcp23017_direction_adapter.h"
#include "ekm014_mcp23017_adapter.h"

// R24: non-actuating, joint MCP23017 register audit for the proposed R23
// port A = four EKM014 control inputs and port B = MDD20A direction.
// These reads do NOT prove physical coil current, motor supply isolation,
// watchdog cut-off or electrical safety. Never use this result as an
// independent-hardware-commissioned flag.
namespace boat_r24 {
static constexpr uint8_t MCP_ADDRESS = 0x27;
static constexpr uint8_t IOCON = 0x0A;
static constexpr uint8_t IODIRA = 0x00;
static constexpr uint8_t IODIRB = 0x01;
static constexpr uint8_t GPIOA = 0x12;
static constexpr uint8_t GPIOB = 0x13;
static constexpr uint8_t OLATA = 0x14;
static constexpr uint8_t OLATB = 0x15;
static constexpr uint8_t EKM_BITS = 0x0F;
static constexpr uint8_t MDD_DIR_BITS = 0x03;

// Only valid after a known MCP23017 POR/reset with IOCON.BANK=0 and
// verified 0x27 address. Failure/NACK always means "not ready".
// GPIO readback is NOT a substitute for independent current feedback.
inline bool registerOutputsDisabled(Mcp23017RegisterBus &bus,
                                   uint8_t address=MCP_ADDRESS) {
  if (address != MCP_ADDRESS) return false;
  uint8_t iocon=0xFF, dira=0xFF, dirb=0xFF;
  uint8_t olata=0xFF, olatb=0xFF, gpioa=0xFF, gpiob=0xFF;
  if (!bus.readRegister(address, IOCON, iocon) ||
      !bus.readRegister(address, IODIRA, dira) ||
      !bus.readRegister(address, IODIRB, dirb) ||
      !bus.readRegister(address, OLATA, olata) ||
      !bus.readRegister(address, OLATB, olatb) ||
      !bus.readRegister(address, GPIOA, gpioa) ||
      !bus.readRegister(address, GPIOB, gpiob)) return false;
  return iocon == 0x00 &&
         dira == 0xF0 && dirb == 0xFC &&
         (olata & EKM_BITS) == 0 &&
         (olatb & MDD_DIR_BITS) == 0 &&
         (gpioa & EKM_BITS) == 0 &&
         (gpiob & MDD_DIR_BITS) == 0;
}

// Bench/diagnostic initialization ONLY, with BOTH high-current 3S paths
// physically disconnected. The caller must not turn on MDD20A supply or
// EKM014 VH just because this returns true. Separate adapters set low OLAT
// values before changing the selected pins from inputs to outputs.
inline bool prepareDiagnosticDisabled(Mcp23017RegisterBus &bus,
                                      Mcp23017DirectionAdapter &motors,
                                      Ekm014Mcp23017Adapter &releases) {
  const bool first = motors.prepareDisabled();
  const bool second = first && releases.beginDisabled();
  const bool audited = second && registerOutputsDisabled(bus);
  if (!audited) {
    // Best-effort only. If I2C failed, this is NOT physical power removal.
    (void)releases.failClosed();
    (void)motors.failClosed();
    return false;
  }
  return true;
}
} // namespace boat_r24
