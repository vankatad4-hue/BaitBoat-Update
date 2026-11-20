# E22-900T22S test configuration R1

## Regional channel
Use the initial candidate channel **868.125MHz** (E22-900T22S channel 18; its documented channel step is 1MHz and frequency base is 850.125MHz). This is within the 868.0–868.6MHz sub-band with 25mW ERP and ≤1% duty cycle or compliant polite spectrum access under current harmonized conditions. Confirm Bulgarian national implementation and device conformity before RF field use.

The E22 channel spacing means it cannot select the 869.4–869.65MHz 10% duty-cycle sub-band's 869.525MHz center. Therefore the backup protocol must be airtime-limited to the applicable 1% band unless a different radio/module is selected.

## Initial identical module settings (both endpoints)
- Module address: 0x0000
- NETID: 0x00
- UART: 9600 baud, 8N1
- Air data rate: 62.5kbps (REG0 value 0x67)
- Sub-packet: 32 bytes (REG1 bits7:6=11)
- Output power: 10dBm (REG1 bits1:0=11); re-evaluate ERP with antenna gain/feed loss
- Channel: 0x12 (868.125MHz)
- Transparent mode; repeater and LBT off initially. LBT setting is not a substitute for duty-cycle enforcement unless the selected access technique meets the regulatory rules.
- RSSI byte append disabled.
- M0/M1: normal mode 0/0; configuration mode M1=1 M0=0. (M1=M0=1 is sleep.)

Configuration write command (9600 8N1, configuration mode):
`C0 00 09 00 00 00 67 C3 12 00 00 00`
Expected response:
`C1 00 09 00 00 00 67 C3 12 00 00 00`
Then read back with `C1 00 09` and compare registers. Exact command/reply and M0/M1 mode confirmed from E22 manual; don't leave module in config mode.

## Air-time policy
- Duty cycle: 1% on channel 18 (868.125MHz).
- Maximum allowed TX airtime budget: 36 seconds per hour per transmitter, subject to national rules.
- Initial conservative estimated airtime per 32-byte frame: 25ms until measured with actual configured module. At that estimate, one TX every 3s = 0.83% duty; each remote and boat transmitter must track separately.
- Default LoRa traffic profile: safety commands/low-rate control/telemetry only; maximum 0.33Hz per endpoint while estimate remains 25ms. No continuous sonar, map or high-rate joystick stream. STOP/RETURN do not bypass duty-cycle budget.
- Measure module air-time/packet delay and use the measured worst-case duration plus margin. Persist duty-cycle accounting across reboot before outdoor operation; current prototype limiter is RAM-only and is not regulatory-ready.

## Electrical
- E22 VCC from BAT_PROT 1S rail (2.7–5.5V operating range), local 100µF+10µF+100nF; exact cap selection/ESR validate. 10dBm config reduces power; verify rail under TX.
- UART is 3.3V logic.
- UART bridge/direct UART: 9600 8N1.
- AUX readiness used for nonblocking TX and configuration handover.


## Datasheet basis
The E22-900T22S manual gives the channel formula 850.125MHz + CH×1MHz, 900MHz channels 0–80, UART baud and air-rate settings, 32-byte subpacket option, and 10dBm power option. The initial channel18 setting is 868.125MHz; the 869.4–869.65MHz center cannot be reached using this module's 1MHz channel spacing.
