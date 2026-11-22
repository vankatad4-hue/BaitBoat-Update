# Boat R24 — boot/reset output-state safety and shared MCP23017 register audit

**Date:** 2026-11-22  
**Status:** NON-ACTUATING DESIGN / HOST REGRESSION ONLY; physical evidence NOT_RUN  
**Hardware:** R20-selected ESP32-S3 DevKitC-1, Waveshare MCP23017 0x27, Cytron MDD20A, Electrokit EKM014. **No added components, PCB or BOM substitution.**

## Why this follow-up exists

R23 documents the J1/J3 GPIO allocation and 64 logical net connections but does not establish safe behavior when one module boots, resets or loses power before another. The two existing MCP23017 adapters configure GPA0–3 for four EKM014 inputs and GPB0–1 for the two MDD20A DIR pins. R24 adds a joint **read-only output-disabled register audit** and a bench-only initializer which calls the existing low-latch-before-output routines on both banks.

**This is not a physical watchdog, independent electrical cutoff, current-zero sensor or coil pulse limiter.** It must not be used to set `independentHardwareCommissioned=true` or to connect MDD20A motor power / EKM014 VH.

## Manufacturer-supported facts (not physical acceptance)

1. Microchip MCP23017: POR resets I/O directions to inputs (IODIRx = 0xff); OLAT and GPIO are distinct, and reading an output latch cannot prove current through a load. The R24 static checker expects IOCON.BANK=0, the default documented register map. Firmware cannot assume it can recover an arbitrarily reconfigured bank without a known reset/controlled initialization.
2. Electrokit EKM014: the manufacturer states each active-high IN input has an **internal 120 kΩ pull-down**. Thus an un-driven input should be biased LOW on the specified board, but the weak bias is **not** evidence of protection against a short, interference, a failed driver, or an uncontrolled logic rail. The 10 A slow-blow fuse is not a 6 A coil pulse-time limiter.
3. Cytron MDD20A: PWM and DIR are logic inputs; onboard manual test buttons can drive the module and its supply lacks reverse-polarity protection. Neither MDD20A's startup PWM state nor behavior under reversed/partial supplies is accepted without actual measurement. The MDD20A 20 A/channel rating does not change the shared 20 A BMS limit.

References: [Microchip MCP23017](https://www.microchip.com/en-us/product/MCP23017), [Electrokit EKM014](https://docs.electrokit.com/en/modules/EKM014/), [Cytron MDD20A](https://my.cytron.io/p-20amp-6v-30v-dc-motor-driver-2-channels).

## Proposed boot sequence with motor/coil HIGH-CURRENT paths physically disconnected

1. Keep `3S_MOTOR` to MDD20A and `3S_SOLENOID` to EKM014 VH **unconnected**. Apply only individually current-limited logic supplies and verify the USB/5V and regulator ramp sequence. This is a controlled bench procedure, not an autonomous startup implementation.
2. On a known expander POR/reset, confirm Waveshare 0x27 address straps and RESET high, 3.3V I2C signal levels, no shorts and no unexpected high values at the four EKM inputs or two MDD direction inputs.
3. ESP GPIO39/40 must remain inactive while the peripheral starts. MDD PWM boot/reset levels and pulse widths require oscilloscope validation and a physically independent motor-supply-off state; no PWM is emitted by R24.
4. Execute `Mcp23017DirectionAdapter::prepareDisabled()` then `Ekm014Mcp23017Adapter::beginDisabled()`, both already enforcing OLAT LOW *before* changing the selected pin directions to outputs.
5. The new `boat_r24::registerOutputsDisabled()` checks IOCON 0, IODIRA 0xF0, IODIRB 0xFC, both selected OLAT masks = 0, and both selected GPIO readbacks = 0. Any read failure, port fault, unexpected direction, nonzero command or chip reset fails the check.
6. The helper `prepareDiagnosticDisabled()` runs the existing initializers plus that joint audit with BEST-EFFORT low commands on failure. An I2C NACK can leave a previous physical output energized. This helper is **not wired into the diagnostic sketch or physical output authority** and is never an independent safety mechanism.
7. Only a separately qualified physical isolation/STOP/independent watchdog/current-zero/pulse limit can permit subsequent dummy-load stages. Previous unsigned R20/R23 flags, uncommissioned ADC and power-gate holds remain unchanged.

## Reset/fault acceptance matrix — all physical tests NOT_RUN

| Scenario | Software expected | Mandatory physical evidence |
|---|---|---|
| MCU boots while MCP unpowered | No output authorization | Scope PWM, EKM INx, MDD supply and EKM VH = isolated |
| MCP powers before ESP | IODIR reset as inputs; R24 audit false | Observe 120 kΩ pull-down performance and no load voltage |
| ESP reset / brownout mid-request | R24 audit alone cannot cut power | Independent watchdog/cutoff visibly removes rated DC power |
| MCP reset while ESP stays alive | Joint audit becomes false until reinitialization | Confirm no coil energization or MDD motion |
| I2C NACK, SDA stuck, partial write | Adapter becomes unready; best-effort software OFF | Verify independent STOP/power path under actual fault |
| MDD test button pressed | Outside software authority | Ensure buttons cannot energize motors during normal operation without intended authorization |
| EKM INx stuck high / EKM MOSFET short | Register state may misleadingly look OFF | Independent current-zero monitor and rated cutoff must detect/mitigate |
| Radio both lost | R78 immediate software STOP, conditional RTH after 10 s | Separately verify physical watchdog and STOP during MCU lockup |
| Battery ADC invalid or critical | R79 fail-closed software STOP | Verify 3.3V/5V/analog sequencing and shutdown on actual wiring |

Every row is an instruction for later verification, NOT a passed measurement.

## Code/test changes

- `Boat/source/mcp23017_joint_boot_audit_r24.h`: combined disabled-register audit and diagnostic preparation (does not power hardware).
- `Boat/tests/boat_r24_mcp_joint_boot_test.cpp`: simulated POR, low-latch initialization, release/direction activity, unexpected direction, stuck-high, IOCON bank mismatch, I2C NACK and recovery.
- `.github/workflows/boat-host-tests.yml`: new R24 C++11 host regression step.

R20 parts and R23 pin connections remain unchanged. All R23 32 physical sign-off rows remain NOT_RUN. Boat VER014 / Remote VER028 unchanged. No official release promotion or live-load permission.

## Next gate

Purchase/photograph exact board/carrier revisions, then record PWM39/40 boot waveforms and actual EKM IN1–4 / MDD DIR1–2 idle voltages with only logic supplies. Resolve and test independently rated STOP/watchdog/isolator and current-zero feedback before any high-current motor/solenoid path is connected. Do not infer manufacturer connector cavity order from logical net names.
