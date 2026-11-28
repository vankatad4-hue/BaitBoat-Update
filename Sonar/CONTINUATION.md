# Sonar coprocessor continuation

**Updated:** 2026-11-28
**Target:** owner's classic ESP32-DevKitC V4, 4 MB flash; Open Echo TUSS4470
Shield 002 remains an uncommissioned candidate.

## Software prepared

- Standalone disarmed PlatformIO target with upload denied.
- Bounded 5,224-sample ADC capture buffer, optional ESP-IDF ADC continuous/DMA
  adapter, and a pure echo threshold classifier. The adapter is not instantiated
  by firmware. Requested sample rate, voltage mapping, sample timing, and
  transmit-time origin are not validated on the board.
- Side-effect-free TUSS4470 SPI frame codec. Separate read/write functions
  validate six-bit addresses and form odd-parity frames; reads use the required
  zero data byte. Response decoding validates parity and exposes raw status,
  VDRV-ready, fault flags, device state, and returned data.
- Boat accepts bounded NMEA DBT through an isolated UART0/USB-CDC build profile.
  The receive gate remains disabled until Boat GPIO21 and the coprocessor TX
  level are physically checked and signed. Sonar GPIO candidates are documented
  only; no sensor, SPI, ADC, UART transmitter, Wi-Fi, or transducer drive is
  enabled in the default firmware.

## Next steps

1. Inspect the exact Shield 002 revision, transducer, jumpers, power input and
   output levels; confirm all proposed ESP32 pin connections electrically.
2. Build the disarmed target and isolated ADC API profile, then review any
   compiler/API failures before changing hardware gates.
3. After electrical signoff, add a gated SPI transport that sets the verified
   mode, stays at or below 500 kHz, raises NCS between every frame, and rejects
   parity/status faults.
4. Measure ADC interval, overflow behavior and burst-time alignment; validate
   detector settings against known distances before enabling DBT output.

## Protocol reference

The frame layout, read dummy byte, response status assignments, and SPI edge
behavior are taken from the [TI TUSS4470 datasheet](https://www.ti.com/lit/ds/symlink/tuss4470.pdf).

Map integration remains postponed. No local build, test, or hardware validation
is claimed for the current frame codec.
