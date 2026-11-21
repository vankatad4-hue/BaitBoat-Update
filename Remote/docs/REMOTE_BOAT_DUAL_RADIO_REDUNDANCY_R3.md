# Dual-radio pairing and antenna decision R3

## Pairing
The initial backup link uses identical EBYTE E22-900T22S UART LoRa modules at remote and boat. The Heltec Tracker candidate's integrated SX1262 remains unused because raw SX1262/E22 interoperability was not proven.

## Remote antenna assignment
- LEFT external antenna: nRF24 PA/LNA 2.4GHz.
- RIGHT external antenna: E22 EU868 50Ω.
- ESP32-S3 Wi-Fi/Bluetooth uses onboard antenna.
- Boat has separate 2.4GHz nRF and 868MHz E22 antennas, separated from each other, GNSS, motor leads, high-current switching and metal obstructions.

## Initial E22 settings
- channel18=868.125MHz, network/address0, air62.5kbps, packet32,10dBm, transparent, UART9600 8N1.
- Module VCC: remote dedicated TPS61023 5V boost; boat regulated5V electronics buck.
- UART/M0/M1/AUX are3.3V logic. Remote SC16IS750 bridge@0x48. Boat direct UART pins TBD.
- Remote CS8501 5V reserved for controller/display; E22 converter separate. Do not parallel boost outputs.
- Estimated25ms airtime; one frame/3s≈0.83% in1% sub-band. RAM-only limiter is not field-compliant.

## Backup traffic
STOP, RETURN_HOME, restricted manual steering, heartbeat/low-rate telemetry only. No sonar, new mission, bait/hook release. Boat executes local failsafe if both links lost. Return-home only with valid navigation and system-health checks.

## Regulatory
868.125MHz is in1% duty sub-band. Confirm Bulgaria/CEPT band limits, EIRP/ERP, antenna gain/cable loss, bandwidth, duty/LBT/AFA requirements and module compliance. 10dBm module output is a test setting, not regulatory certification.

## Test
Identical modules/settings, antennas attached, close-range first. Verify config readback, UART logic, packet CRC/auth, latency/packet loss, failover, power rail droop and antenna isolation. No field operation until legal airtime accounting and authenticated session/key are completed.

## R61 purchasing and assembly gate (2026-11-22)
The specified **E22-900T22S is a bare SMD radio module (26×16mm)**, not a connectorized plug-in board. No custom PCB is permitted: purchase only with a verified ready-made SMD-to-header carrier or professionally assembled off-the-shelf breakout that preserves RF grounding, regulated power, AUX, M0, M1, UART and antenna connection. Neither an adapter SKU nor pair RF interoperation has been verified. **Do not order an E22-900T22D-V2 as an undocumented substitute.** If necessary, formally revise both endpoints to matching DIP variants with configuration/readback and RF bench tests first. EU868 field use remains blocked on regulatory airtime and authenticated radio sessions.
