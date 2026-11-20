# E22-900T22S Bench Configuration Decode R1

Checked: 2026-11-20. Applies only if the purchased module is the exact E22-900T22S variant compatible with this register layout. Confirm marking and readback.

## Current profile byte string
`C0 00 09 00 00 00 67 C3 12 00 00 00`

The first 3 bytes are command, start address, register count; following bytes configure the module.

| Register | Byte | Decode |
|---|---:|---|
| ADDH | 00 | Address high |
| ADDL | 00 | Address low |
| NETID | 00 | Network ID |
| REG0 | 67 | UART 9600 (011), 8N1 (00), air rate 62.5kbps (111) |
| REG1 | C3 | 32-byte subpacket (11), RSSI append disabled (0), reserved bits zero, 10dBm output (11) |
| REG2 | 12 | Channel 18 |
| REG3 | 00 | Candidate default option bits; verify exact variant/manual before relying on this register |
| REG4 | 00 | Candidate default option bits; verify exact variant/manual before relying on this register |
| REG5 | 00 | Candidate default option bits; verify exact variant/manual before relying on this register |

Frequency calculation documented for this module family: 850.125 MHz + channel × 1 MHz. Channel 18 gives 868.125 MHz.

## Test code behavior
Both E22 bench sketches send only fixed `LINK_PROBE` diagnostic frames, CRC checked, no authenticated commands or actuator payloads. Remote sends at a 5-second interval; boat sends at 7.5-second interval. This is bench-only traffic and not a duty-cycle-compliant field implementation.

## Hardware check gate
1. Verify module product marking and version.
2. Read configuration in configuration mode and compare response byte-for-byte.
3. Confirm antennas attached before RF test.
4. Start short range and 10dBm profile.
5. Record serial logs, current draw, packet reception and losses.
6. Do not operate in the field until Bulgaria/EU regulatory parameters, antenna gain/EIRP, duty cycle and LBT/AFA requirements are reviewed.

## Manufacturer references
- EBYTE E22-xxxT22S user manual: https://resource.heltec.cn/download/Wireless_Tracker/Wireless%20Tracker%201.1.pdf (Heltec hardware manual link; verify actual EBYTE manual from exact product vendor)
- EBYTE product page: https://www.ebyte.com/product/1587.html
- EBYTE E22-xxxT22S manual PDF: https://www.ebyte.com/Uploadfiles/Files/2024-1-5/2024151430332094.pdf

This note is an engineering decode, not a compliance approval.
