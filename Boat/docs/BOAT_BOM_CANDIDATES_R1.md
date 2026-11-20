# Boat Hardware BOM Candidates R1

| Ref | Qty | Candidate | Purpose | Validation |
|---|---:|---|---|---|
| U_CTRL | 1 | Heltec Wireless Tracker V1.2 (ESP32-S3 + UC6580 GNSS) | Controller/GNSS | Exact SKU, pinout and power input |
| U_NRF | 1 | nRF24L01+ PA/LNA | Primary radio | Module and 3V3 burst current |
| U_E22 | 1 | EBYTE E22-900T22S UART LoRa | Backup radio | Same SKU/config as remote; 5V supply and 10dBm settings |
| ANT_NRF | 1 | 2.4GHz 50Ω antenna | Primary RF | Coax, mounting, enclosure |
| ANT_E22 | 1 | EU868 50Ω antenna | Backup RF | ERP/EIRP with gain/feed loss |
| U_MOTOR | 1 | Cytron MDD20A candidate | Dual 550 brushed motor driver | Board 88.9×78.74mm; exceeds carrier footprint; current/thermal test |
| U_5V | 1 | 3S-to-5V buck, ≥2A peak candidate | Controller + LoRa | Load measurement, converter efficiency/thermal |
| U_12V_SOL | 1 | 3S buck-boost 12V candidate, only if coil voltage range requires | Solenoid power | Coil current and voltage tolerance |
| Q_BAIT | 1 | Logic N-MOS low-side | Bait solenoid | Current, Rds(on) at 3.3V, thermal |
| Q_HOOK | 1 | Logic N-MOS low-side | Hook solenoid | Current, Rds(on) at 3.3V, thermal |
| D_BAIT | 1 | Flyback diode | Bait coil clamp | Coil current and release time |
| D_HOOK | 1 | Flyback diode | Hook coil clamp | Coil current and release time |
| F_MAIN | 1 | Main fuse candidate ≤ pack/wire rating | Pack short protection | Coordinate BMS and motor current |
| F_M1/F_M2 | 2 | Motor branch fuse candidates | Motor driver branches | Select after startup/stall current |
| F_ELECTRONICS | 1 | 2A candidate | Logic/E22 branch | Measure peak |
| F_SOL | 2 | Coil branch fuse candidates | Solenoids | Coil current/duty |
| J_PACK | 1 | Keyed high-current connector | Battery pack | Rating/cable gauge |
| J_MOTOR | 2 | Locking high-current connectors | Motors | Wire gauge, current |
| J_E22 | 1 | Module/carrier connector | UART LoRa | Exact module pinout |
| J_NRF | 1 | 8-pin radio connector | nRF SPI/control/power | Keying/pinout |
| J_SOL | 2 | 2-pin locking connectors | Bait/hook coils | Rated for measured coil current |
| TP | set | Test pads | BUS_3S, 5V, 12V_SOL, 3V3, UART, SPI, PWM/DIR, GND | Test access |
