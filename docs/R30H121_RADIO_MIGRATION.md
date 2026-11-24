# R30H-121A — direct physical access inventory

Source: Boat/source/boat_dual_radio_port.cpp at audit time. Production owner unchanged.

| Driver operation | Occurrences |
|---|---:|
| nrf_.begin | 1 |
| e22_.begin | 1 |
| nrf_.update | 1 |
| nrf_.txPending | 6 |
| nrf_.ready | 2 |
| nrf_.lastTxSuccess | 1 |
| nrf_.receiveFrame | 1 |
| nrf_.sendFrame | 5 |
| e22_.update | 1 |
| e22_.transportFault | 1 |
| e22_.receiveFrame | 1 |
| e22_.ready | 2 |
| e22_.sendFrame | 4 |

## Migration contract

- Initialization, polling and RX move to exclusive worker ownership; RX is copied with capture time.
- Readiness and pending state become freshness-bounded copied health, never direct driver reads.
- Send calls require explicit asynchronous admission and terminal outcome; queue acceptance is not delivery.
- Handshake and application tickets need distinct domains; root-key authentication stays on application owner.
- STOP priority and retry must survive queue saturation and generation invalidation.
- No production activation until regression and physical commissioning gates pass.
