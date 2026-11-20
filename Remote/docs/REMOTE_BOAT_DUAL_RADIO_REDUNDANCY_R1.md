**SUPERSEDED:** Current pairing and settings are in `REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md`. R1 contains historical candidate details; do not use its module power or channel suggestions as the current design.

# Dual-radio redundancy architecture R1 — Remote + Boat

**Decision update (2026-11-20):** Add a LoRa backup link alongside the nRF24L01+ PA/LNA primary link. This is a paired system: the remote AND the boat must have LoRa modules for a usable redundant connection. This document supersedes any statement that the nRF24 is the only radio.

## Radio roles

| Link | Hardware | Band | Purpose | Traffic |
|---|---|---|---|---|
| Primary | nRF24L01+ PA/LNA at both ends | 2.4 GHz | Responsive driving and normal control | Fast control, telemetry, acknowledgements; sonar/map data only if bandwidth test supports it |
| Backup | EBYTE E22-900T22S-V2 (SX1262, UART) at both ends — candidate | Configure for EU868 | Safety/redundant link | Heartbeat, link health, low-rate telemetry, reduced-rate control, STOP/RETURN commands |
| Remote Wi-Fi/Bluetooth | Waveshare ESP32-S3 onboard antenna | 2.4 GHz | Setup/updates only | Not boat-control redundancy |

## Module candidate

**Candidate:** EBYTE E22-900T22S-V2, SX1262, UART, 22 dBm-capable, 850–930 MHz family, 16×26 mm module. Confirm exact regional SKU, firmware configuration, connector, supply limits, certifications and availability before ordering. Its RF power must be configured/limited for the selected Bulgarian/EU channel and antenna system; the module's maximum power is not automatically legal.

- UART data interface; module TX → ESP UART RX, module RX ← ESP UART TX (3.3V logic only).
- M0/M1 mode pins: connect to controller GPIOs or strap in a documented fixed mode; do not leave floating.
- AUX: connect to controller input for module busy/ready status; pull-up/logic details per exact module datasheet.
- Power: module version advertises a 5V supply for full output-power capability; allocate a protected, filtered 5V branch only after the Waveshare CS8501 rail is tested under simultaneous display/backlight and radio loads. Expected transmit current around 110mA for the selected candidate (verify actual datasheet/revision).
- Decoupling at module: candidate 100µF bulk + 10µF + 100nF, short low-impedance supply/return; sizing is a bench-validation item.
- UART pins must be assigned only after exact Waveshare revision/header GPIO crosswalk and current GPIO-use audit. Do not choose pins based on presumed free GPIOs.

## Remote antenna assignment (physical layout)

**RIGHT external antenna connector = LoRa EU868 antenna.** Dedicated 50-ohm RF path from the LoRa module's antenna connector to the right-side antenna exit; connector gender and cable type must match module and antenna.

**LEFT external antenna connector = nRF24L01+ PA/LNA 2.4GHz antenna.** Keep RF connectors, coax, and antennas frequency-specific and labelled.

The Waveshare ESP32-S3 Wi-Fi/Bluetooth uses its onboard antenna; no external Wi-Fi antenna connection is planned in this revision. This replaces the earlier concept of a second external antenna for ESP32 Wi-Fi/Bluetooth.

Remote antenna placement: keep both antennas physically separated, orient them to avoid mutual shadowing, and keep them away from display metal, battery cells, wiring, hand/grip, and RF ground discontinuities. Verify antenna tuning/matching and enclosure detuning after mechanical design. Label ports **LEFT—2.4GHz nRF** and **RIGHT—868MHz LoRa**; do not interchange.

Boat: add its own LoRa module and 50-ohm 868MHz antenna as a second RF link. Mount the nRF24 and LoRa antennas away from each other, motors, ESC wiring, battery leads, and metal/carbon structures. Remote-only LoRa cannot provide redundancy without the boat counterpart.

## Regulatory and RF settings

- Use EU863–870 regional operation, not 915MHz US settings.
- Candidate channel plan for initial testing: 868.1/868.3/868.5MHz, subject to exact national/CEPT sub-band restrictions, device compliance, and power/duty-cycle design.
- CEPT/ERC Recommendation 70-03 and ETSI EN 300 220 requirements apply to SRD usage. Channel, EIRP/ERP, duty cycle, bandwidth, antenna gain/cable loss and any LBT/AFA requirements must be checked for the selected sub-band and Bulgaria before field use.
- Do **not** operate at the module's 22dBm maximum by default. Set power so the total radiated power is within the applicable limit (the candidate EU sub-band limits can be far below 22dBm) and implement duty-cycle accounting/airtime limiting. Regulatory settings are a release blocker.
- Use short backup packets, airtime calculator, exponential/backoff retry, and channel occupancy tracking. Do not send continuous joystick stream or sonar data on LoRa.
- A LoRa connection does not guarantee 500m; antenna placement, water-level path, Fresnel obstruction, packet settings, interference and local limits must be field-tested.

## Backup-link protocol and failover behavior

### Radio abstraction
Firmware must use a common LinkManager interface with PRIMARY_NRF, BACKUP_LORA, LINK_NONE, per-link metrics, and a unified command/telemetry packet. Each transport has its own driver/task; do not block LVGL/UI or joystick sampling on radio operations.

### Health metrics
- Primary nRF: rolling packet ACK/heartbeat success, sequence freshness, retry count, RSSI if available, and last-valid timestamp.
- Backup LoRa: module AUX/ready state, packet ACK/heartbeat success, sequence freshness, and last-valid timestamp.
- Never treat a queued TX as a delivered command. Require valid ACK/response or boat-side received sequence telemetry.

### State machine
1. PRIMARY: nRF is active control link; LoRa remains low-duty standby/heartbeat and link-health probe.
2. DEGRADED: primary loss threshold reached; warn UI, reduce control rate and prohibit new mission/release actions until link confidence restored.
3. BACKUP: only enter after authenticated/fresh LoRa response. Use limited safe control profile (slow speed, capped turn rate), heartbeat, telemetry, STOP and RETURN. No sonar streaming.
4. NO_LINK: both links stale. Remote shows link loss; boat firmware executes its own failsafe (stop/return-home only if navigation and system health are valid, otherwise stop motors and await recovery). Never depend on remote link to trigger failsafe.
5. RECOVERY: nRF must pass a configurable streak of fresh ACKs before handover. Switch at a command boundary; avoid oscillation with holdoff/hysteresis. LoRa remains ready during transition.

Thresholds are provisional, to tune in tests: primary heartbeat 20–50Hz; warn after 3 missed heartbeats or ~150–250ms (choose based on measured latency); backup telemetry/heartbeat low rate (e.g. 1–2Hz); backup joystick commands initially 2–5Hz; recovery requires 5–10 consecutive good primary responses and a short holdoff. Validate airtime/duty-cycle before adopting final rates.

### Packet safety
Common packet fields: protocol version, source/destination IDs, message type, sequence counter, timestamp/age, mode, control values, payload length, CRC, and authentication tag. Reject stale, duplicated, malformed, wrong-ID, invalid-mode, or unauthenticated command packets. Add cryptographic authentication/replay protection (e.g. AES-CCM with per-project provisioned key) before unattended control; CRC alone is not security. Duplicate command suppression is required across both radios so a command delivered by both links is executed once. Releases (bait/hook) must never be repeated by retries: command IDs plus boat-side acknowledgement/state confirmation and idempotent command handling.

### Mode rules on backup
- Backup link supports safety actions and low-rate manual steering with speed limits.
- Mission start, bait release, hook release, calibration and firmware update are disabled over backup unless a deliberate, tested policy is later approved.
- If the boat is already running a mission, boat autonomy remains local. Link-loss policy: return home only if GNSS/heading/battery and geofence state are valid; otherwise stop motors and report/hold when link recovers.
- Remote UI clearly identifies PRIMARY/BACKUP/NO LINK and limits controls when on backup.

## Schematic additions (remote)
- Add LoRa module connector/module footprint and 5V branch; UART, AUX, M0, M1, GND, supply and antenna RF connection.
- Candidate nets: LORA_5V, LORA_UART_TX, LORA_UART_RX, LORA_AUX, LORA_M0, LORA_M1, LORA_RF_868.
- Do not connect LoRa to the nRF SPI bus; selected UART module uses serial interface.
- Add ADC monitoring for LORA_5V via divider only if ADC channel budget/voltage range is reviewed; current ADC2 spare channel can be reassigned.
- Power branch must not assume the Waveshare boost can supply the module during simultaneous display/backlight peaks; test before PCB freeze.

## Schematic additions (boat)
- Add matching LoRa UART module, 5V supply/filter, UART/GPIO control, AUX input, M0/M1 and 868MHz RF connector/antenna.
- Boat MCU UART assignment and power capacity require board-level audit.
- Antenna feed and module should be physically separated from motor-current switching and nRF antenna.

## Acceptance tests
1. Verify exact module revision/datasheet and command/config utility.
2. Confirm 3.3V UART logic levels; verify no 5V TX signal into ESP32.
3. Test CS8501 5V rail with display/backlight, nRF burst, and LoRa TX simultaneously; record droop, ripple, temperature and brownout.
4. Validate antenna connectors, matching 868MHz antenna, cable loss, isolation, and enclosure tuning.
5. RF bench test packet delivery/latency/ACKs, then outdoor boat test over water at measured distances.
6. Test failover, failback hysteresis, duplicate/replay/stale packets, STOP/RETURN, release-command idempotency, and loss of both links.
7. Confirm Bulgaria/EU compliance (channel, power, EIRP/ERP, duty cycle, antenna gain and certification) before field operation.
8. Update both Remote and Boat software archives and version numbers only after tested release synchronization.

## References
- EBYTE E22-900T22S-V2 product data (UART, SX1262, 850–930MHz family, 22dBm maximum, 3.3V UART, 16×26mm): https://ebyteiot.com/collections/rf-module-1/products/ebyte-e22-900t22s-uart-5km-range-868mhz-915mhz-22dbm-smd-ce-fcc-lora-wireless-transmitter-rf-module-semtech-sx1262
- CEPT/ERC Recommendation 70-03 active reference: https://docdb.cept.org/document/845
- ETSI EN 300 220-2: https://www.etsi.org/deliver/etsi_en/300200_300299/etsi_en_30022002/03.03.01_60/en_30022002v030301p.pdf
- LoRa EU863-870 channel/rate reference: https://lora-alliance.org/wp-content/uploads/2021/05/RP002-1.0.3.pdf
