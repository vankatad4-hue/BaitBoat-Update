# R30H-124N — Remote ordinary and mission TX migration contract

Preparation only. Production remains legacy-owned; physical workers disabled.

Core 1 owns session keys, sequence, ACK tracking, mission upload/control and shared I2C. Core 0 requests polling. RemoteAsyncTx is the only ticket and generation issuer. Centralize completion draining: STOP/heartbeat, ordinary and mission consumers must not independently drain one result queue. Extend result metadata with message kind, logical transaction/request ID, generation and original deadline. A stale generation or fault cancels logical pending work; already admitted RF may complete but cannot authorize subsequent action.

| Traffic | Primary | Backup |
|---|---|---|
| STOP/heartbeat | Existing critical scheduler | Existing critical scheduler |
| CLEAR_STOP, releases, ordinary mode change | Existing authenticated primary interlocks | Denied |
| Manual motor/sticks | Fresh primary lease, neutral/rearm gates; replace obsolete samples | No broadened authority |
| RETURN_HOME | Existing authorization, valid Home | Existing backup policy only |
| Mission PREPARE/upload/START | Existing primary authorization, matching transaction | No broadened permission |
| Mission PAUSE/CANCEL | Existing mission/transaction checks | Existing backup policy only |
| Mission feedback | Authenticated peer, sequence, transaction and request correlation | Same validation, no primary movement permission |

Retain STOP/heartbeat priority; bound ordinary reservations and retain original deadlines. Never retry ambiguous actuator release as a new action ID. Distinguish queue acceptance, local transport evidence, peer ACCEPTED/EXECUTED, timeout and unknown. Mission retry accounting uses correlated local evidence; authenticated Boat ACK/feedback remains separate. Session changes, STOP, link fault and retirement revoke pending work. No automatic rearm or legacy fallback.

Next: verify host suites and both PlatformIO targets, implement typed completion dispatcher, primary-only authorized ordinary traffic, mission transaction scheduler; test heartbeat cadence, busy links, duplicate/stale ACK, cancellation, failover and session rollover. Physical activation requires hardware acceptance.
