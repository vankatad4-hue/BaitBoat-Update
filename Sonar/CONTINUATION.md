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
- The same host and four target checks passed on the current branch head in
  [#46134623965](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46134623965).
- After narrowing the CI path filters to firmware, tests, and build configuration,
  the host test and all four target profiles passed in
  [#46135033752](https://github.com/vankatad4-hue/BaitBoat-Update/actions/runs/46135033752).
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
4. The Boat-to-Remote profile path now has a bounded 128-bin display-envelope
   protocol. It splits five bins across each 8-byte authenticated telemetry
   payload (26 fragments/profile), sends one fragment only in the existing
   idle primary heartbeat gap, abandons stale transfers, and never uses E22.
   Remote validates fragment shape and assembles out-of-order chunks before
   publishing a complete profile. This is not raw capture transfer: 5,224 raw
   12-bit samples would require about 1,959 frames at four data bytes/frame.
   There is no profile ACK/retry yet, and the sonar coprocessor does not yet
   produce/feed the 128-bin envelope into Boat's new publication boundary.
5. Remote now retains up to 64 completed profiles and renders their measured
   amplitudes as a scrolling LVGL canvas. The old depth-history graph remains
   the fallback when no profile has arrived. No fish-arch classification is
   implemented; the display shows amplitude only and must not infer fish IDs.
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

## R30H-124W8.5c.2 follow-up audit

The following status was rechecked against the current `platformio-unified-r30h20`
working tree on 2026-11-28:

| Requested item | Current verified state | Remaining gate |
| --- | --- | --- |
| Active sonar acquisition runtime | Not active. `Sonar/src/main.cpp` statically requires the hardware-enable flag to remain zero. ADC DMA capture and the TUSS4470 SPI adapter are compile-gated components, not a running sonar acquisition pipeline. | Exact Shield 002 revision, transducer model/electrical data, VOUT conditioning, register profile, burst control, ADC timing and burst/sample synchronization. |
| Full echo-profile communication | A 128-bin measured-envelope protocol and best-effort primary-link sender/reassembler are implemented. It is display-resolution, not raw ADC capture; no ACK/retry or Sonar-to-Boat profile producer is connected. | Feed calibrated measured bins from Sonar into `TrackerSensorPort::publishSonarEchoProfile`, measure actual airtime, and decide whether reliable retry or raw-profile transport is required. A 5,224-sample 12-bit capture is 7,836 bytes before framing. |
| Remote echogram | Remote reassembles complete profiles and renders a 64-column scrolling amplitude canvas. Fish classification is not implemented; depth-history remains the fallback. | Confirm scale/range alignment and rendering on the Waveshare display. Only connect real acquisition data; do not invent fish arches. |
| Lucky D+T water-temperature interface | Still unidentified from public documentation. Lucky's official FF718-T page describes a cable probe, 200 kHz/45-degree sonar, and water-temperature display, but publishes no temperature-element pinout, sensor type, resistance curve, or protocol. A display feature is not evidence of accessible thermistor wires. | Read the exact transducer and monitor labels/manual, inspect the cable/conductor mapping, then measure or obtain the temperature element specification before implementing a driver. Do not infer a thermistor or ADC connection from “D+T”. |
| Dual-core runtime acceptance | Not accepted on hardware. Existing diagnostic code/build coverage checks task affinity and progress in software, but no Boat/Remote board runtime results are recorded. | Run the bench on both physical boards; record core IDs, worker progress/drop counters, radio load, STOP latency, watchdog and heap/stack behavior. |
| GPIO assignments | Candidate maps and compile-time uniqueness/range checks exist for the DevKitC V4 Sonar coprocessor; Boat and Remote have their own candidate maps. Physical audits remain unsigned, so assignments are not finalized for wiring/activation. | Confirm exact board SKUs/revisions and connector-to-GPIO continuity, then sign each endpoint's pin audit. Keep all peripherals disabled until that audit is recorded. |

### Temperature-source evidence

Lucky's official [FF718-T product specification](https://www.luckysonar.com/products/fish-finder-pitcher-ff718-t)
states the cable probe operates at 200 kHz with a 45-degree beam and lists water
temperature on the display. The official [manual download page](https://www.luckysonar.com/pages/lucky-download)
provides the FF718LIC-T manual but does not expose an electrical pinout in its
publicly indexed text. The exact purchased D+T sensor model is still not recorded
in this repository. Therefore no temperature driver can safely be selected yet.

### GPIO sign-off rule

The present pin maps are allocation proposals, not wiring authorization. “Final”
requires board revision, physical header pin, ESP32 GPIO number, signal direction,
voltage domain, shared-bus membership, boot-strapping impact, and continuity check
to agree for every connected function. A compile-time distinct-pin check alone
cannot establish that mapping.

### Latest software follow-up — 2026-11-28

Commit `6ed24d03` adds sonar-profile scheduling to the gated asynchronous Boat
radio-owner path as well as the active legacy scheduler. It preserves the same
primary-only idle-gap policy, snapshots profile metadata under the profile lock,
and advances the fragment index only after local PHY completion. The branch was
pushed and the local tracking ref matched GitHub at this commit. GitHub combined
status had not reported checks when this checkpoint was written; the local
workspace has no PlatformIO or C++ compiler, so this follow-up is awaiting CI
verification. This scheduler parity does not activate the async radio owner,
sonar acquisition, or any physical output.

