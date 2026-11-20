# Boat Hardware Block and Wiring Diagrams R1

## Power tree
```mermaid
flowchart LR
 PACK[3S4P Pack] --> BMS[20A BMS candidate]
 BMS --> FMAIN[Main fuse]
 FMAIN --> SWITCH[Retained master switch]
 SWITCH --> BUS[BUS_3S]
 BUS --> MOTOR[MDD20A candidate/external motor driver]
 BUS --> BUCK5[5V electronics buck]
 BUCK5 --> CTRL[Heltec Tracker controller]
 BUCK5 --> E22[E22 LoRa backup module]
 BUS --> SOL12[12V solenoid rail: regulate if coil requires]
 SOL12 --> BAIT[Bait low-side MOSFET]
 SOL12 --> HOOK[Hook low-side MOSFET]
 CTRL --> NRF[nRF24 primary]
```

Current limits, branch fuses, converter sizing, coil voltage and motor stall current are not finalized.

## Control and outputs
```mermaid
flowchart TD
 MCU[Heltec Tracker MCU] --> NRF[nRF24: SPI/CE/CSN/IRQ]
 MCU --> E22[E22 UART: RX/TX/AUX/M0/M1]
 MCU --> GNSS[Integrated GNSS]
 MCU --> SENS[I2C heading/sensors]
 MCU --> PWM1[PWM1 + DIR1]
 MCU --> PWM2[PWM2 + DIR2]
 PWM1 --> MDRV[Dual H-bridge driver]
 PWM2 --> MDRV
 MDRV --> M1[Left 550 motor]
 MDRV --> M2[Right 550 motor]
 MCU --> GATE1[Bait gate]
 MCU --> GATE2[Hook gate]
 GATE1 --> Q1[Low-side MOSFET + flyback]
 GATE2 --> Q2[Low-side MOSFET + flyback]
 Q1 --> BAITCOIL[Bait solenoid]
 Q2 --> HOOKCOIL[Hook solenoid]
```

The controller pin numbers are not frozen; use the exact Tracker schematic. Motor driver module is larger than the measured 77×79mm carrier area and requires separate placement or a mechanical revision.
