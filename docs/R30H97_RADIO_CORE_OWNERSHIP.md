# R30H-97 — radio core ownership migration
Status: source audit, production worker disabled.
## Observed coupling
- Boat/source/boat_dual_radio_port.cpp: update() combines PHY polling, authenticated session, mission controller and application callbacks. NRF uses SPI; E22 uses HardwareSerial(1); GNSS uses UART2 and sensors use Wire in firmware/Flytec_Boat.ino.
- Remote/source/boat_radio.cpp: both SC18IS606 nRF SPI bridge and SC16IS750 E22 UART bridge use Wire; nRF CE uses MCP23017 on Wire. Remote UI/application reads and mutates boatState, telemetry, mission and ACK state through boatRadio APIs. TF SPI is not the nRF bus.
- Both update functions drain RX with while(receiveFrame(...)); bounded per-cycle processing must be verified before worker migration.
## Required split
Core 0 exclusively owns physical radio driver initialization, polling, TX admission and completion. Core 1 retains authentication/session, mission, STOP application, ACK construction, UI and state. Transfer copied frames and TX completion records with bounded queues; no raw pointers to mutable application state. STOP authentication remains on Core 1; raw RX is not a STOP authority. Preserve correlated ACK and real status ordering.
Remote Wire access requires a single bus ownership strategy covering all clients, including display/touch/input; do not assume Arduino Wire locking is sufficient for multi-transaction bridge operations. Keep existing hardware gates and upload denial.
## Acceptance
1. Inventory every Wire client and transaction sequence; choose bus owner or bounded transaction arbitration.
2. Define frame/completion records and bounded RX/TX backpressure; prove STOP latency under flood.
3. Separate PHY polling from protocol update, test fake drivers and preserve existing single-core path.
4. Compile both targets; physical RF and actuator acceptance remains mandatory.
