# R8 Heltec onboard SX1262 engineering checkpoint
2026-11-21 — source integration, NOT hardware acceptance.

Single-controller Heltec Tracker plus separate Cytron MDD20A, no RP2040. Boat
integrated SX1262 replaces external E22/E227; nRF24+ PA/LNA stays primary.
Four separate FW0630 bait/hook solenoids and physical independent power cutoff
remain requirements. Sonar is on hold with no model, wiring or code chosen.

## GPIO
External native demands: nRF CE/CSN 2, I2C SDA/SCL 2, MDD20A PWM/DIR 4,
independent watchdog WDI and actuator permit 2 = **10**. Former E22 UART
TX/RX count of two removed. Optimistic unverified V1-family candidate pool
11 => count-only margin 1; conservative pool 5 => shortfall 5. None of the
pins are signed/commissioned. GPIO8..14 remain occupied internally by LoRa,
NOT available external carrier signals. Published **V1.1 only** SX1262 pins:
NSS8 SCK9 MOSI10 MISO11 RESET12 BUSY13 DIO1=14
(https://github.com/zephyrproject-rtos/zephyr/blob/main/boards/heltec/heltec_wireless_tracker/doc/index.rst).
Verify exact V1.2 GNSS, TFT, USB, flash, strap, ADC and header wiring.
External nRF may share onboard SPI 9/10/11 only if accessible and verified:
CSN separation, transaction locking, bus clock/mode and startup CS HIGH.

## Radio implementation and blockers
Boat/source/heltec_sx1262_link.* contains a preliminary RadioLib 7.3.0
raw 32-byte packet transport. ALL GPIO values default -1. BOARD_AUDIT_SIGNED,
PEER_PHY_VERIFIED and EU868_APPROVED default zero in boat_radio_hw_config.h,
so no LoRa hardware initialization or RF TX occurs by default. The Boat
diagnostic sketch also cannot arm actuators. The Remote remains unchanged
and has an E22 transparent UART radio via SC16IS750. The raw SPI SX1262
is **not proven RF-compatible** with the E22 firmware. Must either validate
actual transparent E22 on-air framing experimentally, or redesign remote
backup to a suitable SPI/raw SX1262 with its own GPIO audit. Until then,
backupReady remains false and is not a reliable failover.

Provisional 868.1 MHz, BW125 kHz, SF9, CR4/5, private sync 0x12, 8-symbol
preamble, 32 bytes gives **247 ms airtime**, not the previous speculative
25ms. A provisional 1% envelope imposes 24.7-second pacing per packet.
Current 3-second backup heartbeat and 6-second freshness are thus NOT
realistic as continuous backup at this PHY. EU frequency/power/duty/LBT
and persistent accounting through resets remain open regulatory gates.
No claim of continuously usable manual LoRa fallback.

## Next electrical steps
Get authoritative V1.2 schematic/real PCB measurements and sign usable
pinout; confirm radio pairing; complete independent latched TPS3431
STOP/UVLO/overcurrent/thermal power cutoff; capture four separately fused
low-side MOSFET/flyback coil drivers and hardware one-hot pulse limitation;
resolve 15 open ERC violations in preliminary KiCad R7; then PCB/DRC,
dummy-load commissioning, and physical acceptance. No power/coil values
are final without motor stall, pack and coil measurements. Remote version
028 / Boat version 014 remain unchanged; prospective 029/015 withheld.

Native safety/coil schematic capture contract: [BOAT_SAFETY_SOLENOID_CAPTURE_R8.md](BOAT_SAFETY_SOLENOID_CAPTURE_R8.md). This is a drafting specification and has not reduced the existing ERC violations.

### R8 integrated SX1262 verification (2026-11-21)
- Boat ESP32-S3 **target compile #27 PASSED** on source SHA `9c96f937011b4af15179b2cb3c172df2bf0b1b83`; https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167311 . Arduino-ESP32 core 3.0.2, RF24 1.4.11, RadioLib 7.3.0. Program 417,037/1,310,720 bytes (31%), global memory 22,120/327,680 bytes (6%, 305,560 remaining). Diagnostic only: GPIO/radios uncommissioned, actuator power disconnected.
- Boat portable host tests **#64 PASSED** on same source SHA: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167361 . New test includes 10-signal GPIO feasibility, LoRa airtime and 1% interval model; simulated checks only.
- Snapshot ZIP CI #1110 **PASSED** on source SHA 9c96f937: https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/44868167366 . Newer docs-only snapshots also generated; official release markers remain 028/014 and their immutable releases were NOT authorized.
- Code fix after transport insertion: shared SPI selects inactive before bus init, then RF24 init and SX1262 init; RadioLib constructor requires `SPIClass&`, corrected. Commits `ca82b7dc`, `9c96f937`.
- R8 superseding purchasing list and pin schedule: `Boat/docs/BOAT_BOM_CANDIDATES_R8.md`, `Boat/docs/BOAT_HARDWARE_PIN_CONNECTIONS_R8.csv` (commit `2ed54e3`). Electrical capture drafting spec: `Boat/docs/BOAT_SAFETY_SOLENOID_CAPTURE_R8.md` (commit `7bb4968`). These are *not* native KiCad power/coil circuits.
- The **remaining blocker** is an authoritative V1.2 signed pin map and verified Remote-to-Boat backup LoRa PHY. Remote still runs E22/SC16IS750; Boat raw SX1262 is intentionally disabled. 10 vs optimistic 11 GPIOs is count-only. Independent physical fault latch, all four coil driver circuits and measured power budget are NOT complete; R7 KiCad has **15 open ERC violations**. No PCB/DRC, hardware acceptance, or sonar integration. Retain single Heltec + direct MDD20A without RP2040.
