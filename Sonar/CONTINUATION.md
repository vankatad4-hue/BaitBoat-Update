# Sonar coprocessor continuation

**Updated:** 2026-11-28
**Target:** owner's classic ESP32-DevKitC V4, 4 MB flash; Open Echo TUSS4470
Shield 002 remains an uncommissioned candidate.

## Software prepared and verified

- Boat and Remote PlatformIO targets passed in Actions run
  [#46133216294](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46133216294).
- The standalone DevKitC V4 Sonar target and all four PlatformIO profiles
  (base, ADC, SPI, UART) passed in Actions run
  [#46133216583](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46133216583).
- After adding the compile-time GPIO allocation checks, the Sonar host test and
  all four target profiles passed again in
  [#46134352872](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46134352872).
- Acquisition, stable-echo detection, invalid/incomplete capture handling,
  DBT formatting, and Boat's DBT parser round-trip passed the Sonar host test
  in the same workflow. This is simulated software behavior, not a sensor test.
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
- Provisional ESP32-DevKitC V4 assignments are now explicit in
  `src/sonar_pin_map.h`, documented in `README.md`, and protected by
  compile-time range, strap-pin, input-only, and distinct-pin checks. Boat's
  separate GPIO21/J3-18 UART0 RX candidate is enabled only in its USB-CDC
  compile profile. No pin audit has been signed.
- Boat accepts bounded NMEA DBT through an isolated UART0/USB-CDC build profile.
  The receive gate remains disabled until Boat GPIO21 and the coprocessor TX
  level are physically checked and signed. Sonar GPIO candidates are documented
  only; no sensor, SPI, ADC, UART transmitter, Wi-Fi, or transducer drive is
  instantiated in the default firmware.

## Open integration tasks

1. Inspect and record the exact Shield 002 revision, Lucky wired D+T model,
   capacitor jumpers, power input, signal levels, transducer wires, and
   temperature-sensor interface. The manufacturer lists the wired 718LIC D+T
   sensor as 200 kHz/45-degree and requires a compatible fish finder, but its
   public page does not document the temperature element pinout or protocol.
2. Complete ADC/TUSS runtime setup only after the exact transducer frequency,
   shield jumpers, VOUT conditioning, IO modes, and burst controls are confirmed.
   In particular, capture is not synchronized to transmit yet: measured sample
   interval and burst-origin sample coordinate are mandatory inputs to depth
   detection.
3. Measure ADC interval, overflow behavior, SPI mode/rate/NCS timing, and
   signal levels on the target. Validate thresholds and depth against known
   distances before enabling DBT output.
4. Define a bandwidth-bounded, authenticated full-range echo-profile transfer
   protocol between Boat and Remote. The current radio telemetry is 8 bytes per
   authenticated frame and only carries a compact depth summary; no profile
   transfer/reassembly path exists. Do not bypass STOP/control priority or
   blindly flood the backup E22 link.
5. Replace the Remote's depth-history line with a scrolling echogram only after
   real time/amplitude profile columns reach the display. Draw measured return
   amplitudes; do not fabricate fish arches from a depth-only reading.
6. Add a Lucky D+T temperature driver once its actual thermistor/interface
   specification is supplied or measured. Boat/Remote already have a bounded
   water-temperature telemetry/UI field, but the active Boat sensor port has
   no selected water-temperature source.
7. Run the dual-core runtime bench on the actual Boat/Remote boards and verify
   core affinity, task progress, queue/drop counters, radio timing, STOP
   latency, and temperature/memory behavior. Build success alone is not runtime
   acceptance.

## Protocol reference

The frame layout, read dummy byte, response status assignments, and SPI edge
behavior are taken from the [TI TUSS4470 datasheet](https://www.ti.com/lit/ds/symlink/tuss4470.pdf).
The mode 1 candidate follows the [Open Echo TUSS4470 sketch](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/arduino/TUSS4470_arduino/TUSS4470_arduino.ino),
which uses 1 MHz; this project's transport lowers that rate to 400 kHz to stay
within TI's 500 kHz maximum.

The current [Open Echo `settings.h`](https://github.com/Neumi/open_echo/blob/main/TUSS4470_shield_002/arduino/TUSS4470_arduino/settings.h)
defaults both transducer drive and the TUSS band-pass filter to 40 kHz; its
200 kHz filter setting is only a comment. Do not reuse that preset for the
candidate 200 kHz Lucky wired D+T transducer. TI requires the receive filter to
match transducer center frequency, while Open Echo Shield 002 also requires
matching capacitor-jumper selection. A real profile must satisfy both.

Map integration remains postponed. Firmware builds and host tests are verified,
but no sensor voltage, timing, transducer, UART wiring, depth, temperature,
echogram, or physical dual-core runtime acceptance is claimed.
