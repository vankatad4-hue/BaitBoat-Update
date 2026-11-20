# Physical Button Map R6

Front-facing view. **Both button banks are arranged four columns across × two rows high.**

## Left bank (4×2)
| Row | Col 1 | Col 2 | Col 3 | Col 4 |
|---|---|---|---|---|
| Top | L1 STOP | L2 MANUAL/AUTO | L3 HOME | L4 SAVE WAYPOINT |
| Bottom | L5 WAYPOINT SELECT | L6 MAP FULLSCREEN | L7 SONAR FULLSCREEN | L8 MENU/SETTINGS |

## Right bank (4×2)
| Row | Col 1 | Col 2 | Col 3 | Col 4 |
|---|---|---|---|---|
| Top | R1 H-L (left hopper) | R2 H-R (right hopper) | R3 K-L (left hook) | R4 K-R (right hook) |
| Bottom | R5 RELEASE STATUS | R6 RELEASE LOCK/STATUS | R7 MAP/SONAR SPLIT | R8 BACK/CANCEL |

## Confirm / safety
Dedicated physical OK/confirm key is separate from the two banks and power key. Hold release key ≥1200ms, release, then hold OK ≥800ms within 4000ms. STOP, stale/unverified safety telemetry, primary-link loss, timeout or cancel aborts. No actuator release over backup LoRa.

Release commands are independent; no combined hopper or hook release. Boat-side actuator execution and telemetry interlocks remain incomplete; release stays fail-closed.

Preview: [R6 SVG](../CAD/REMOTE_CONCEPT_R6.svg).


## R6 physical CAD detail
- Separate OK/confirm switch is at lower-center of the panel (provisional location), distinct from the touchscreen and power key.
- Four right-bank release buttons receive preliminary raised collar geometry in CAD. Collars are not a tested safety feature; retain electronic staged-confirm and boat-side interlocks.
