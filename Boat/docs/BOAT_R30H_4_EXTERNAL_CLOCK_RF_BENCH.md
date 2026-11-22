# R30H-4 — external common-clock RF-only bench capture

**Status 2026-11-22: prepared, no physical RF measurements.** This is an external-instrument evidence workflow, not permission for radio transmission, motor power, solenoid operation or water trials. Boat VER014 / Remote VER028 unchanged.

## Delivered engineering assets

- [Empty common-clock CSV](BOAT_R30H_4_LOGIC_CAPTURE_TEMPLATE.csv) — one electrical edge per row; no fabricated timings.
- [Unsigned evidence manifest](BOAT_R30H_4_CAPTURE_MANIFEST_TEMPLATE.json) — mode UNSPECIFIED, no claims that boards or signal voltages were inspected.
- [Ten-channel physical probe crosswalk](BOAT_R30H_4_RF_ONLY_PROBE_CROSSWALK.csv) — all ten physical mappings NOT_READY. No debug GPIO pins selected.
- [Host-only analyzer](../tools/analyze_r30h4_common_clock_capture.py) — validates external-clock timestamps, event names/edges, order, missing source evidence, origin, raw capture SHA-256, firmware identities, sampled rate and proof that high-current supplies are disconnected.
- [Host negative tests](../tests/test_r30h4_common_clock_capture.py) — empty evidence, synthetic only, forged real mode, inconsistent clocks, USB-local timestamps, reversed or duplicate events, and 9,999,999-us premature RTH marker.

**Important:** this analyzer cannot establish that a waveform truly came from hardware. Even with complete manifest, it **never returns physical PASS**. It returns NOT_RUN (exit 3), INVALID_CAPTURE (exit 2), or EXTERNAL_CAPTURE_REVIEW_REQUIRED / SIMULATED_NONCERTIFIABLE (exit 4). There is deliberately no successful signoff exit.

## What the external probes would show

| Probe | Direct electrical observation | What it CANNOT prove |
|---|---|---|
| STOP_BUTTON_EDGE | Remote STOP-button edge | Firmware accepted/queued STOP |
| REMOTE_NRF_SPI_CSN | Remote nRF SPI transaction | Authenticated STOP packet or radio delivery |
| BOAT_NRF_IRQ | Boat nRF IRQ | HMAC accepted or output zero |
| REMOTE_E22_UART_TX_START | Remote E22 UART start | That it was STOP or an over-air packet |
| BOAT_E22_UART_RX_START | Boat E22 UART start | Authentication, safety decision, current zero |
| RF_CUT_TRIGGER | Common-clock RF fixture trigger | Radio truly blocked without separately verified isolation |
| BOAT_STOP_INTENT_MARKER | **Not physically implemented** | No such GPIO/testpoint today |
| BOAT_LOGICAL_OUTPUT_OFF_MARKER | **Not physically implemented** | No current-zero/physical STOP proof |
| BOAT_LAST_AUTH_RX_MARKER | **Not physically implemented** | Valid authenticated last packet |
| BOAT_RTH_ENTRY_MARKER | **Not physically implemented** | Real RTH motion |

Boat USB-local R30H-2 and Remote R30H-3 local timing traces have **independent clocks**. They must not be subtracted for end-to-end STOP time. An analyzer with one actual common timebase must record both devices simultaneously. A matching clock_domain text label alone cannot convert unrelated clocks into a real shared clock.

## RF-only bench sequence — execution remains blocked

1. Inspect actual Waveshare Standard and Boat ESP32-S3 DevKitC-1 board variants, SC16IS750 carrier, E22-900T22S exact revisions and both nRF modules and antennas. Sign exact connector orientation. The R30H-1 22 physical gates remain NOT_RUN.
2. Keep one common 3S4P Boat battery architecture, but **physically disconnect MDD20A VM and EKM014 VH high-current load feeds** for RF-only work. This does not add a second switched power domain. No motors, coils or dummy loads.
3. Use an appropriately current-limited logic-only test supply after verifying 3.3-V-safe UART/IO and the actual logic-analyzer input ratings. E22 VCC may be 5V while its UART IO must be ESP32-compatible. Never connect unverified 5V signals to ESP32/analyzer.
4. Verify the exact E22 pinout and actual PHY config readback, legal EU868 antenna/channel/EIRP/duty conditions, and measure actual 32-byte E22 airtime. The firmware's 25ms airtime and 1% accounting are **provisional**. Never run PA/LNA modules without appropriate antennas/RF terminations.
5. After physical level/regulatory/pairing approval, use only an RF-only non-actuating bench build with HMAC secrets provisioned outside GitHub. Scope STOP button, nRF SPI/IRQ, E22 UART on both ends, and separate common-clock RF-block trigger.
6. Perform at least 30 trials per applicable R30H STOP/loss case, covering E22 3000ms pacing phases. Record original instrument export, SHA-256, same-clock normalized CSV, power/antenna photos, module model, both firmware commit IDs and a witness review.
7. For authenticated STOP or last-auth-to-RTH claims, a **separately reviewed, physically verified debug marker** is mandatory. No authenticated STOP/RTH marker is wired/commissioned now. UART activity alone never means valid STOP or RTH.
8. Current-zero, real motor/coil turn-off, independent hardware STOP/watchdog and one-solenoid-at-a-time release hardware are a **later** stage after all R30A safety interlocks are physically approved. No physical power application at R30H-4.

## Common-clock file schema and CLI

CSV columns: capture_id,trial_id,case_id,clock_domain,time_us,platform,event,edge,origin,notes. Timestamps are integer microseconds from the actual external recorder, not Remote/Boat local millis. For source origin: EXTERNAL_ANALYZER only when physically captured by one instrument; SYNTHETIC for code-fixture tests. Never enter USB_LOCAL as a cross-board timestamp.

On a Windows PC, compute a raw-file SHA-256 with PowerShell Get-FileHash -Algorithm SHA256 and put that result in the manifest capture_file_sha256. Then run:

    python Boat/tools/analyze_r30h4_common_clock_capture.py actual_external_capture.csv actual_external_manifest.json

A valid hardware-capture manifest requires actual instrument model/asset ID/sample rate, 40-hex Remote and Boat firmware commits, actual E22 and ESP32 board revisions, evidence of disconnected VM/VH and a physical preflight reference. Even perfect metadata still produces **REVIEW_REQUIRED**, not physically certified STOP.

## Safety and next step

R30H-3 16 acceptance gates NOT_RUN, R30H-1 22 module signoffs NOT_RUN, R30H 22 RF trials NOT_RUN; physical common-3S STOP/current-zero R30A gates remain OPEN. The software E22 STOP may wait up to ~3s for its next permitted TX window, and total-link stale may be recognized up to ~6s after the last valid E22 frame. These are software-derived exposures, not measured RF safety limits.

**Next R30H-5 software task:** specify an isolated, non-actuating authenticated-path GPIO/test marker and its unverified pin-allocation constraints for eventual common-clock capture, then continue fault simulation. **Actual RF testing and STOP timing still require purchased/inspected modules and safe bench apparatus.** Sonar unselected; versions unchanged.
