> **HISTORICAL R5 — superseded by R8 integrated-SX1262 decision.** Do not purchase the Boat E22 or wire its UART/M0/M1/AUX. Current BOM: BOAT_BOM_CANDIDATES_R8.md; current pin schedule: BOAT_HARDWARE_PIN_CONNECTIONS_R8.csv. The remaining historical notes below are retained for traceability.

# Active Boat BOM — R5 single-controller revision
This is an engineering candidate BOM, not a purchasing or fabrication authorization. No companion MCU, companion supply or companion antenna. Retain existing hull, motors, battery indicator, master switch.

| Ref | Qty | Component | Status |
|---|---:|---|---|
| U_CTRL | 1 | Heltec Wireless Tracker V1.2 | Selected; exact board pinout unresolved |
| U_MOTOR | 1 | Cytron MDD20A | Selected; separate mount |
| M_L/R | 2 | Existing 12 V 550 brushed motors | Retain; measure startup/stall currents |
| U_NRF | 1 | nRF24L01+ PA/LNA | Selected |
| U_E22 | 1 | E22-900T22S | Selected |
| ANT_NRF/E22 | 2 | Matching RF antennas | Validate RF and regulatory settings |
| U_IMU | 1 | GY-91 MPU-9250/AK8963 + BMP280 | Selected candidate; exact module verification |
| U_ADC | 1 | ADS1115 | Candidate battery monitor |
| U_5V | 1 | 3S-to-5V buck >=2A peak | Candidate; size after load test |
| U_RF_3V3 | 1 | Dedicated 3.3 V regulated rail if necessary | Pending burst load/rail review |
| U_WD | 1 | TPS3431 | Candidate independent watchdog; timing and pin circuit not frozen |
| U_IO | 0 or 1 | Non-programmable GPIO expansion | Optional; only if pin audit requires; part TBD |
| U_GATE | TBD | Independent power cutoff/latch and hardware one-hot/pulse logic | Component ratings and actual schematic pending |
| SOL_LH/RH | 2 | FW0630 009 DC12V | Retain; characterize individually |
| SOL_LK/RK | 2 | FW0630 001 DC12V | Retain; characterize individually |
| Q_SOL | 4 | MOSFET and gate drive | TBD after coil and thermal measurements |
| D_SOL | 4 | Flyback/clamp network | TBD after energy and release-time measurement |
| F_SOL | 4 | Individual branch protection | TBD |
| J_SOL | 4 | Keyed two-pin rated coil connectors | TBD |
| F_MAIN/F_MOTOR/F_LOGIC | 3 | Coordinated fusing | Pack/BMS/current/wire measurements required |
| U_SOL12 | 0 or 1 | 12 V buck-boost | Only if coil pack-voltage tolerance requires |
| R_DIV/C_DIV | 3 | 47k,10k 0.1%,100nF | Candidate ADS1115 divider; protection separate, not finalized |

See [R5 design](BOAT_BOARD_INTEGRATION_R5.md) and [R5 connector schedule](BOAT_HARDWARE_PIN_CONNECTIONS_R5.csv). No high-current stage on the 77x79 mm control carrier. No electrical-rule or physical test acceptance.
