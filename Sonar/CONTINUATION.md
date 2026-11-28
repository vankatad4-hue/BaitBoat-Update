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

## R30H-124W sonar data-link implementation — 2026-11-28

- Added `Common/source/sonar_data_protocol.h`, a bounded version-1 UART frame
  format with CRC-16, strict frame lengths, 128-bin echo-profile payloads, and
  signed centi-degree water-temperature payloads. This is sensor transport only;
  it does not claim a selected/working Lucky D+T temperature driver.
- Sonar's gated UART adapter can now send complete echo profiles and water
  temperature samples. Boat's optional UART adapter decodes these frames and
  passes them into the existing `TrackerSensorPort` profile and temperature
  publication boundaries, alongside the existing NMEA DBT depth path.
- Extended the Sonar host contract test with profile round-trip, signed
  temperature payload, sequence/sample, and CRC-corruption cases. CI now runs
  that test with the canonical Common source include path and triggers on this
  protocol header.
- Verification is pending for these edits. This workspace has no usable native
  C++ compiler, Python runtime, or PlatformIO command; run the Sonar host and
  target workflow after syncing this commit. No board upload or hardware test
  was attempted.
- The acquisition firmware remains statically disarmed. The transport API is
  not yet called by a live acquisition runtime because burst synchronization,
  TUSS register profile, analog conditioning, and sample calibration remain
  unsigned. The UART data link also remains behind physical pin audits. Remote's
  existing 128-bin telemetry/echogram path is unchanged and still awaits a
  produced profile from the Sonar board.
- Sonar sleep is still ESP32 light sleep only. The current disarmed build has no
  active acquisition drivers to stop; future commissioned drivers need explicit
  stop/restart hooks before the power command can sleep/wake the fully active
  sonar hardware. The Remote piezo output remains disabled pending its GPIO
  audit. Map integration remains postponed.

## R30H-124W8.5c.2 follow-up audit

The following status was rechecked against the current `platformio-unified-r30h20`
working tree on 2026-11-28:

| Requested item | Current verified state | Remaining gate |
| --- | --- | --- |
| Active sonar acquisition runtime | Not active. `Sonar/src/main.cpp` statically requires the hardware-enable flag to remain zero. ADC DMA capture and the TUSS4470 SPI adapter are compile-gated components, not a running sonar acquisition pipeline. | Exact Shield 002 revision, transducer model/electrical data, VOUT conditioning, register profile, burst control, ADC timing and burst/sample synchronization. |
| Full echo-profile communication | A 128-bin measured-envelope protocol and best-effort primary-link sender/reassembler are implemented. It is display-resolution, not raw ADC capture; no ACK/retry or Sonar-to-Boat profile producer is connected. | Feed calibrated measured bins from Sonar into `TrackerSensorPort::publishSonarEchoProfile`, measure actual airtime, and decide whether reliable retry or raw-profile transport is required. A 5,224-sample 12-bit capture is 7,836 bytes before framing. |
| Remote echogram and circle view | Remote reassembles complete profiles and renders a 64-column scrolling amplitude canvas. The current UI change overlays a compact green range-footprint circle at the echogram's upper left, with range-only peak rings and a GY-91 heading line when fresh. The 45° Lucky D+T angle is still a candidate pending exact sensor verification. A single beam has no echo azimuth, so pings are complete rings and do not claim fish direction or location. | Review the new preview and verify display scaling on Waveshare. No build has yet verified this UI edit. See [UI preview](../Remote/docs/SONAR_DISPLAY_PREVIEW.svg). |
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
and advances the fragment index only after local PHY completion. The code was
verified by Boat and Remote host tests and both primary PlatformIO builds on
commit `6cc974ac` (GitHub Actions run `46138381538` for Boat/Remote builds;
`46138381595` and `46138381601` for host tests). The local workspace has no
PlatformIO or C++ compiler. This scheduler parity does not activate the async
radio owner, sonar acquisition, or any physical output.

The circular Remote view was CI-verified on commit `9c8b5ab0`. The current
unverified UI edit places that circle over the echogram and marks strong
range-profile peaks with full dashed rings. The echoes have no measured bearing,
so the rings intentionally do not position or identify fish. The circle's
footprint diameter is estimated as `2 × depth × tan(beam angle / 2)`; the
candidate 45° angle and exact transducer remain unverified. A software-standby
button is labeled `Sonar Off` / `Sonar On` and suppresses Remote-side profile
processing and clears the local plot; the status explicitly says it is
Remote-only. It does not put the Sonar ESP32 to sleep.
The existing Sonar link is one-way UART1 TX to Boat RX, and no
Remote→Boat→Sonar control command is implemented. The Sonar firmware continues
to compile with hardware activation hard-disabled. For future sleep/wake
control, ESP32 UART wake is available in light sleep (UART wake does not wake
deep sleep); this requires a return UART line, reviewed Boat/Sonar GPIO
assignments and a framed command/ack path. No sonar power switch is assigned.

The current Waveshare ESP32-S3-Touch-LCD-4.3 standard board has no documented
onboard speaker/audio codec; its carrier plan only reserves an optional buzzer
connector without an assigned, verified output pin. Sonar profiles produce
amplitude peaks, not a fish classifier, so any alert must be called an echo
candidate. The requested [Pixabay sonar ping](https://pixabay.com/sound-effects/film-special-effects-sonar-89448/)
is listed under Pixabay Content License; no MP3 has been copied into firmware.
The refreshed `Remote/docs/SONAR_DISPLAY_PREVIEW.svg` is a static design
preview, not a live capture or build-verified output. Audio hardware, sample
choice/license handling, alert thresholds/debounce, and device sleep/wake remain
open work; do not treat Remote standby as a sonar battery-saving mode.
The proposed control and passive-piezo integration sequence is documented in
[`Remote/docs/SONAR_POWER_AND_PING_ALERT_DESIGN.md`](../Remote/docs/SONAR_POWER_AND_PING_ALERT_DESIGN.md).

