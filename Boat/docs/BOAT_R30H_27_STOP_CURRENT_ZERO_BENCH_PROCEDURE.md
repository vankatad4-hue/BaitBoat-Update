# R30H-27 — Controlled STOP/current-zero physical bench acceptance procedure

**Status:** `BLOCKED / NOT_RUN` — commissioning plan only; **do not connect actuator power yet**.  
**Architecture:** ONE shared protected 3S4P battery distribution; parallel MDD20A VM and EKM014 VH taps. No second motor/coil supply is authorized.  
**Evidence register:** [BOAT_R30H_27_FEEDBACK_PROVENANCE_MATRIX.csv](BOAT_R30H_27_FEEDBACK_PROVENANCE_MATRIX.csv).  
**Existing acceptance gates:** R30H-10 H10-06/09/10/13/14/15/17 and R30A independent physical review.

## Gate A — design and deenergized inspection

Verify actual purchased board revisions, manufacturer pinout, conductor polarity, DC voltage rating, worst-case motor stall/coil current including the 6A-class single-release pulse, shared 3S BMS/fuse constraints, battery isolation and safe grounding. Record part numbers and source datasheets; keep MDD20A VM and EKM014 VH **physically isolated from the battery**. Do not treat an MCP23017 output readback as current measurement, an INA231 BUS voltage as shunt-current evidence, or a software watchdog as an independent cutoff. Confirm real DC disconnect, external watchdog and independent pulse limiter selection before moving to the next gate.

## Gate B — source provenance and safe observer checkout

For each of the 18 rows in the evidence CSV, record the physical signal's **real** manufacturer, reader hardware revision, connector pad, optional pull-ups, measured logical polarity, isolated source ID, original acquisition timestamp and raw-capture path. Specifically verify separate measured motor and coil current-zero feedback, independent common-disconnect auxiliary contact, stop-loop contact, hardware watchdog contact and independent limiter output. Record measurement instruments and their calibration/serial numbers. Unassigned GPIO1/2/41/42 are NOT approved sensor pins. Do not fabricate them to fill missing rows.

Perform electrical continuity and instrument input safety checks **without high-current motor/coil power**. Confirm the reader cannot drive contactor, PWM or EKM014 output; only physical, separately sourced signals may be elevated to an independent-feedback claim after reviewer approval.

## Gate C — reviewed low-energy dummy-load tests only

Once qualified physical independent cutoff, protection and observer interfaces have been installed and a second reviewer has approved the setup, use suitable safeguarded dummy loads and external instrumentation. On the **same external clock**, capture separately:

1. Remote STOP button contact event, authenticated Boat receive event and command ID ACK
2. Actual actuation of the independent STOP/watchdog cutoff
3. Actual voltage at both MDD20A VM and EKM014 VH common-bus taps
4. Auxiliary contact feedback separate from commanded coil/relay control
5. Physical current through each motor path and the active release output
6. Independent one-hot limiter response and post-OFF current-zero at each release channel
7. Raw timestamps and observed reader age, rather than the software poll timestamp

**Never use ACK or `STOP=1` alone as the zero-current endpoint.** For every waveform distinguish STOP issued, STOP received, cutoff opened, current decayed and independent zero observed.

## Gate D — fault matrix and reviewer decision

Record results for ordinary STOP, RF loss/restore, MCU freeze, reset/brownout, watchdog fault, open STOP circuit, stuck output/welded switching element, incorrect feedback polarity, stale/missing source and solenoid retrigger/simultaneous channels. All outcomes require safe physical isolation of **both** load groups; a logic-only STOP without actual load disconnection is a **FAIL**. Confirm independent pulse termination and current-zero even if the MCU software stops progressing. Do not proceed to water testing until dummy-load signoff is complete.

For H10-09 acquire **30 or more raw STOP-button trials** with consistent external clock. This does not waive independent hardware acceptance. Do not invent latency thresholds; determine them against actual component characteristics and sign them.

## Evidence and result entry

Populate `BOAT_R30H_27_FEEDBACK_PROVENANCE_MATRIX.csv` only with actual measurements. Each marked PASS must identify a device revision, calibrated instrument, external raw trace and SHA-256 digest, test conditions, reviewer and review date. Attach an independently reviewed DC contactor data sheet and cutoff power-off test report. If evidence is unavailable, retain `NOT_RUN` or `BLOCKED` — never turn an unmeasured status GREEN.

No GPIO signoff, hardware enable flag, official VER number or firmware upload permission may be changed merely because the host provenance audit or PlatformIO target compilation passes.
