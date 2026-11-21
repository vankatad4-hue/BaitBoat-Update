# R48 — five-stage integration and physical acceptance matrix

Selected ESP32-S3-DevKitC-1-N32R16V. R45 proposed pin schedule is NOT an approved wire-for-wire drawing; GPIO47/48 1.8V and octal-memory reservations apply.

| Requested stage | Source status | Remaining release gate |
|---|---|---|
| GPIO/wiring | R45 provisional J1/J3 schedule, GPS UART115200, GY-91/M100 I2C, nRF SPI, E22 UART, MDD20A PWM/DIR, watchdog/STOP feedback | Inspect physical PCB revision, exact E22 SKU and EKM014 control levels, confirm every header, electrical drawings, cable fuses and connectors |
| Physical drivers | MDD20A Arduino-ESP32 v3 LEDC stage exists, BoatPhysicalDriver interface exists; R48 physical feedback gate added | Build adapter using actual sampled hardware inputs; integrate EKM014 qualified output module and pulse/current-zero feedback; bench verify without actuator power first |
| Independent safety | R48 gate requires STOP loop, independent watchdog contact, two power contactor feedback contacts, zero-current, battery, signed isolation acceptance | Select qualified COTS parts with DC interruption rating above measured stall/coil load, verify wiring and physical opening under stuck-MCU conditions; no unsigned substitutes |
| Automatic compass figure-eight | R43 sensor capture and R46 heading-progress supervisor exist | GPS trajectory control, authenticated Remote Start/Abort, calibration quality thresholds and persistent CRC/rollback storage; no autonomous motor energizing until independent geofence and safety accepted |
| Integrated testing | R45 diagnostic target compile pass, R47 host tests pass; R48 tests added to CI | Full Boat and Remote target compile, fault-injection integration, real nRF/E22 RF peer interoperability, GPS fix, RTH, STOP and release physical tests, water acceptance |

## Independent safety procurement acceptance criteria
1. Physical normally-closed emergency STOP circuit directly cuts both motor and solenoid DC contactor coils independently of ESP32 software; latching reset and auxiliary feedback.
2. Independent external watchdog module: requires correctly timed heartbeat and opens safety relay if heartbeat missing/stuck. Prove MCU lockup and brownout response. Never count ESP32 internal task watchdog as sole protection.
3. Factory-built DC-rated motor and solenoid contactors, rated for worst-case measured motor stall and four solenoids, with suppression and monitored auxiliary contacts. Contactors must be rated for DC breaking, not merely AC.
4. Hardware pulse-limit / one-hot coil interlock or certified independent controller preventing any two EKM014 solenoids being energized concurrently and limiting pulses even if MCU fails.
5. Real current-zero and rail-voltage sensing, fused power distribution, properly sized 5V and 3.3V rails and battery undervoltage isolation. Select only after measurements; no fictitious exact SKU ratings.

## Physical commissioning (owner / qualified technician)
1. With motor/coil supply **disconnected**, continuity, ground, connector keying, 1.8V pin exclusion, 3.3V GPIO levels, no USB backfeed, logic rail ripple and boot-state GPIO checks.
2. Exercise physical STOP, watchdog expiry, reset/brownout and contactor feedback fault injection with dummy loads; confirm contacts physically open.
3. Verify I2C IDs, UBX NAV-PVT checksums, actual GNSS accuracy and dual compass orientation. Verify signed calibration storage and recovery before autonomous use.
4. With propellers removed and restrained, characterize MDD20A directions/PWM/current limits and single EKM014 pulses, zero-current feedback, thermal rise and fused fault behaviour.
5. Authenticate nRF and E22 remote peers, test radio dropouts, anti-replay and link priority with outputs disabled. Verify loss-of-link RTH only when GNSS/compass/geofence valid; otherwise fail to independent STOP.
6. Perform controlled water tests with physical retrieval and independent emergency STOP; validate turning radius, GPS figure-eight, 16 waypoints, hopper/hook release, override and home.
7. Archive measured results, target build logs, BOM invoices, manufacturer PDFs and wiring photos; then publish next verified release and ZIP. No acceptance may be marked PASS based on source code alone.

**Sonar remains deferred. No physical tests were performed by ChatGPT. Boat VER014 remains official.**
