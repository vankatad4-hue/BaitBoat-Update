# R30H-118L Boat PHY ownership contract
Production remains single-owner. Mock target is unverified. No flash.
## Core 0 future exclusive operations
- NRF24Link begin/update/sendFrame/receiveFrame and all getters, including txPending, lastTxSuccess, counters and ready.
- E22UartBoardPort begin/update/sendFrame/trySendFrame/receiveFrame and all getters, including transportFault, configured, txPending and txOutcome.
- SPI/RF24 and UART/GPIO access must not overlap legacy owner calls. Initialization may block; measure separately.
## Core 1 retained operations
BoatDualRadioPort session/sequence/authentication, dedupe, command queue, STOP ACK/status outboxes, mission, RTH and application actuator decisions.
## Required copied messages
Per-link RX frame + capture timestamp; readiness/fault snapshot; TX admission result with generation/epoch/serial; separate terminal PHY outcome correlated to original ticket. E22 UART admission is not RF receipt. NRF24 lastTxSuccess is not application ACK.
## Migration blockers
Current prototype has TX admission only, no RX or terminal outcome channel. Dedicated STOP slot can itself saturate with obsolete STOP; owner must retain newest STOP and retry after drain. Generation check cannot revoke in-flight calls. Handshake frames require an explicit pre-session lane/policy. Shutdown must wait for physical driver quiescence, not just mock exit. No production worker enablement until these are implemented and tested. Next R30H-118M: copied PHY RX and outcome contracts, then executable mock regression.
