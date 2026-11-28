# Sonar coprocessor continuation

**Updated:** 2026-11-28
**Target:** owner's classic ESP32-DevKitC V4, 4 MB flash; Open Echo TUSS4470
Shield 002 remains an uncommissioned candidate.

## Software prepared

- Standalone disarmed PlatformIO target with upload denied.
- Bounded 5,224-sample ADC capture buffer, optional ESP-IDF ADC continuous/DMA
  adapter, and a pure echo threshold classifier. The adapter is not instantiated
  by firmware. Requested sample rate, voltage mapping, sample timing, and
  acoustic burst-origin sample coordinate are not validated on the board. The
  detector takes that origin explicitly and computes depth from elapsed samples
  after the burst, instead of treating capture sample zero as transmit time.
- Side-effect-free TUSS4470 SPI frame codec. Separate read/write functions
  validate six-bit addresses and form odd-parity frames; reads use the required
  zero data byte. Response decoding validates parity and exposes raw status,
  VDRV-ready, fault flags, device state, and returned data.
- Compile-gated ESP32 SPI transport using one 16-bit transaction per NCS pulse,
  400 kHz mode 1, a 3 μs NCS-high guard, write-address echo checking, and explicit
  parity/status errors. Its dedicated PlatformIO API-compile environment does
  not arm hardware. The mode and guard remain unverified on the physical shield.
- Compile-gated UART1 transmitter at 9600-8N1 that accepts only a `FOUND` depth
  result, formats a checksummed DBT sentence, and writes it on the GPIO22
  candidate. Its separate pin audit is unsigned and the main firmware does not
  instantiate it.
- Boat accepts bounded NMEA DBT through an isolated UART0/USB-CDC build profile.
  The receive gate remains disabled until Boat GPIO21 and the coprocessor TX
  level are physically checked and signed. Sonar GPIO candidates are documented
  only; no sensor, SPI, ADC, UART transmitter, Wi-Fi, or transducer drive is
  instantiated in the default firmware.

## Next steps

1. Inspect the exact Shield 002 revision, transducer, jumpers, power input and
   output levels; confirm all proposed ESP32 pin connections electrically.
2. Build the disarmed target and isolated ADC/SPI/UART API profiles, then review
   any compiler/API failures before changing hardware gates.
3. Inspect the transport on the exact target; verify mode, rate, CS boundaries,
   NCS-high guard, and signal levels with a scope before signing its hardware
   audit gate.
4. Measure ADC interval, overflow behavior and burst-time alignment; validate
   detector settings against known distances before enabling DBT output.

## Protocol reference

The frame layout, read dummy byte, response status assignments, and SPI edge
behavior are taken from the [TI TUSS4470 datasheet](https://www.ti.com/lit/ds/symlink/tuss4470.pdf).
The mode 1 candidate follows the [Open Echo TUSS4470 sketch](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/arduino/TUSS4470_arduino/TUSS4470_arduino.ino),
which uses 1 MHz; this project's transport lowers that rate to 400 kHz to stay
within TI's 500 kHz maximum.

The current [Open Echo `settings.h`](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/arduino/TUSS4470_arduino/settings.h)
defaults both transducer drive and the TUSS band-pass filter to 40 kHz; its
200 kHz filter setting is only a comment. Do not reuse that preset for the
candidate 200 kHz Lucky D+T transducer. A real profile must match the measured
transducer and shield in both burst frequency and receive-filter frequency.

Map integration remains postponed. No local build, test, or hardware validation
is claimed for the current sonar software.
