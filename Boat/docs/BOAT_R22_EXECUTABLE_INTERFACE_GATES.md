# R22 — Executable fail-closed integration gates
2026-11-21. No custom PCB. No actuator power. This is a host-side test utility, **not** a firmware or hardware validation.

## New executable artifact
`Boat/tools/r22_interface_gate.py` provides:
- Explicit eight-signal direct GPIO budget with conservative reserved v1.1 GPIOs.
- A requirement for authenticated v1.2 schematic and physical board evidence before a pin allocation may pass.
- Duplicate/missing/invalid pin rejection.
- A LoRa configuration equality gate covering frequency, bandwidth, SF, CR, sync word, preamble, CRC, IQ, header, packet mode and opaque E22 firmware framing.
- Mandatory bidirectional physical packet and authenticated ACK/STOP/failsafe tests.

Run locally: `python3 Boat/tools/r22_interface_gate.py`.
This file contains seven unittest cases and can be included in CI. **Tests have not been run on the GitHub runner by this commit.** A passing synthetic test vector is not RF evidence.

## Current blocked inputs
Heltec official published update log currently shows v1.0 and v1.1 details but does not provide an authenticated v1.2 schematic in the reviewed material: https://wiki.heltec.org/docs/devices/open-source-hardware/esp32-series/lora-32/wireless-tracker/hardware-update-log
V1.1 pin references are not valid as v1.2 assignments: https://bridle.tiac-systems.net/doc/4.4/zephyr/boards/heltec/heltec_wireless_tracker/doc/index.html
E22 UART modem manufacturer details: https://www.cdebyte.com/products/E22-900T22S/2 and https://www.micros.com.pl/mediaserver/info-rf%20e22-900t22s.pdf

## Next steps
1. Obtain manufacturer v1.2 PDF schematic and header numbering; if unavailable, request written confirmation from Heltec.
2. Validate safe PWM/DIR/CE/CSN/I2C pins and boot reset states on actual board.
3. Capture E22 configuration readback and RF packet interoperability; do not infer from both using SX1262.
4. Source commercially built independent hardware STOP, monitored contactor, onehot nonretriggerable pulse, POR and current-zero path; current module shortlist does not meet all gates.
5. Only after hardware measurements, freeze branch fuses, connector ratings and wiring harness. Remote VER028 and Boat VER014 remain held; sonar planning only.
