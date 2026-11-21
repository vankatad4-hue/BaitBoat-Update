# R60 — E22 backup radio compatibility and motor power hold (2026-11-22)

## Candidate, NOT approved for purchase
- EBYTE E22-900T22D-V2.0, SX1262-based transparent UART LoRa module, 850.125–930.125MHz range, 868.125MHz default, 2.3–5.5V supply, **3.3V UART IO**, approx. 110mA TX at 22dBm, SMA-K 50-ohm antenna. Official product: https://ebyteiot.com/it/collections/lora-module/products/ebyte-e22-900t22d-v2-0-sx1262-lora-868mhz-wireless-module-uart-22dbm-5km-long-range-fec-sma-k-antenna-rf-wireless-transmitter
- Do NOT interchange E22-900T22D, E22-900T22D-V2.0 and E220-900T22D without a full UART command, RF settings and pinout audit. E220 uses LLCC68 and its defaults may differ. Reference: https://www.ebyte.com/Uploadfiles/Files/2021-7-6/202176188184448.pdf
- The Remote's exact LoRa model and over-the-air modulation/configuration remain unverified. A generic SPI SX127x or SX126x LoRa radio is **not automatically interoperable** with a transparent UART E22 module. Do not buy E22 until both ends have a tested matching PHY, frequency, spreading factor, bandwidth, coding rate, sync word, packet framing, duty cycle and encryption/authentication scheme. RF operation must comply with Bulgarian/EU 868MHz band and power/duty-cycle restrictions; advertised 22dBm is not permission to transmit at that power.

## Cytron MDD20A verified safety facts
- Manufacturer https://my.cytron.io/c-industry/c-dc-motor-drivers/p-20amp-6v-30v-dc-motor-driver-2-channels : two brushed DC motor channels, 6–30V supply, 20A continuous per channel, 60A peak protected, PWM/DIR accepts 3.3V; **NO reverse polarity protection**. A wrong battery connection can destroy the board.
- Test buttons command full speed. Disable motor branch and remove propellers before any bench testing. Driver overcurrent and temperature protections are not a substitute for branch fuse, independent STOP/contactors, watchdog, BMS, and measured stall current.
- Existing 3S4P pack with 20A BMS cannot be assumed to support two 20A-rated channels simultaneously; actual stall/starting current and BMS trips must be measured. Size protection and wire based on real load, not driver maximum rating.

## Gate
**No LoRa or safety-power parts are approved for purchase yet.** Next obtain exact Remote LoRa model, matching RF protocol and EU settings; select verified off-the-shelf DC-rated disconnect and watchdog with safe-on-power-loss wiring, fuses and protected regulators. Sonar deferred, Remote SD only, Boat firmware non-actuating.
