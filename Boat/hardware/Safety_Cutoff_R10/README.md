# R10 hardware safety cutoff — native KiCad candidate, NOT FABRICATION READY

Created 2026-11-21. The standalone editable schematic `Boat_Safety_Cutoff_R10.kicad_sch` introduces a concrete **logic-network** candidate and a load-high-side PMOS topology, but does **not** establish a qualified independent safety system. **Do not energize either motor or solenoids based on this drawing.**

## Physical topology under review

- J1 accepts a protected switched 3S input and PACK_RETURN. F1 main fuse feeds PACK_FUSED; **Q1 P-channel high-side MOSFET** with source at PACK_FUSED and drain at ACTUATOR_BUS; R2 gate-to-source holds Q1 OFF unless a separate **Q2 N-channel** switch pulls its gate low.
- Q2 is driven only by the output of hardwired AND logic `LATCH_Q AND MCU_PERMIT`; R3 pulls Q2 gate low while logic is absent. A **normally-closed mechanical STOP contact S1** is physically in *Q2's return path*: pressing STOP opens the path to ground, turning Q1 off when the candidate Q2 and Q1 are healthy. It also has an independent second-pole `STOP_OK_NC_SECOND_POLE` safe signal into latch clear.
- ACTUATOR_BUS splits into independent fused motor (F2 J5) and coil (F3 J6) rails. J6 `ACT_SOL_POS_AFTER_LATCH_TBD` + PACK_RETURN is the named interface to R9 J1. R4 provisional 100k bleed is **not** a guaranteed hardware energy-discharge path. Stored energy in the MDD20A and flyback must be measured.
- J2 imports a **verified independent** 3V3 logic rail, NOT Heltec 1S charger. J3 imports 10 *conditioned* safety signals: physical STOP second pole, open-drain TPS3431 WDO_N (R1 10k pull-up candidate), UVLO_OK, CURRENT_OK, TEMP_OK, RAIL_OK, MOTOR_IDLE, COIL_IDLE, OUTPUT_FB_OK, POR_OK. The actual comparators, WDO watchdog circuit, UVLO, current/thermal sensing, rail power supply, and POR are **not captured in this R10 sheet**.
- U1–U3 SN74HC08 candidate logic cascade generates `ALL_SAFE_CLEAR_N` as AND of the ten signals (WDO_N high only when pulled up by R1 and no watchdog fault). This drives asynchronous clear `1CLR_N` on **U4 SN74HC74**. U4 has `1D` and `1PRE_N` tied to 3V3, and physically conditioned `ARM_RISING_EDGE_RELEASE_QUALIFIED_TBD` on `1CLK`. A fault immediately forces Q low; restoring safe inputs alone does **not** generate another clock edge. U4's unused second flip-flop is tied to defined states and unused outputs are left NC.
- **IMPORTANT:** J4's ARM input is a requirement for an external release-qualification, debounce, and POR-immune edge-forming circuit. It **is not yet implemented**. A raw ARM switch must NEVER connect directly to U4 CLK, because switch bounce or power-up while held could energize the boat. Even a correct DFF cannot certify a safety system without physical testing and independent power switching.

## Manufacturer reference checks and limitations
- TI SN74HC74: positive-edge triggered D-FF, asynchronous active-low CLR and PRE, operating supply 2–6V; 14-pin package pin numbers 1CLR=1, 1D=2, 1CLK=3, 1PRE=4, 1Q=5, GND=7, VCC=14; tied-off second half candidate. https://www.ti.com/product/SN74HC74
- TI TPS3431 WDO: active-low **open-drain** momentary watchdog fault; requires an external pull-up and asynchronous latched reaction. EN low disables watchdog and must not be allowed as a field safety bypass. https://www.ti.com/lit/ds/symlink/tps3431.pdf
- TI SN74HC08 2-input AND gates: verify physical 14-pin gate mapping and CMOS VIH with the selected 3.3V input conditioning and independent power supervisor before release. https://www.ti.com/product/SN74HC08

## Remaining engineering gates (blocking)
1. Verify 3S pack, BMS trip curve, maximum 550 motor currents and all four coils. Select physically load-rated high-side switch device(s) and layout with current sense and overcurrent cut-off. This **unrated** PMOS candidate can overheat, avalanche or fail short; quantify RDSon, SOA, gate VGS/transients, clamp, fuse/connector and failure mode.
2. Design and capture the **actual** physical ARM clean-edge/release-qualification and non-retriggerable pulse hardware, independent from firmware. Validate physical STOP contact default/failure states and dual-pole consistency. Include actual POR/UVLO/current/thermal comparators, line filtering/Schmitt synchronization and fault propagation.
3. Integrate R7 TPS3431 (15 unresolved ERC), R9 four solenoid stages (standalone 0 ERC) and this R10 candidate in one verified KiCad hierarchy. Ensure `WDO_N`, common return and power nets agree and nothing bypasses physical STOP or latch. No PCB or production DRC yet.
4. Prove fail-safe timing on hardware with both load rails and stored inductive energy, WDI stuck, WDO fault, loss of rail, brownout, STOP held, ARM stuck, Q1 short fault, MCU stuck outputs, etc. No RAM simulator proves electrical fault isolation.

**No RP2040, no rudder, no Boat E22, no sonar hardware/firmware.** Boat controller remains single Heltec Tracker with nRF primary, onboard SX1262 as currently disabled backup pending Remote E22 compatibility; MDD20A direct two PWM+two DIR. Firmware remains intentionally diagnostic, no physical GPIO enabled. Release Remote VER028 / Boat VER014 unchanged.

## Project directory
- `Boat_Safety_Cutoff_R10.kicad_sch`: independent candidate logic/high-side circuit; no qualified footprints.
- `FlytecR10.kicad_sym`: local editable symbol definitions; `sym-lib-table`.
- `BOAT_SAFETY_GATE_BOM_R10.csv`: references and open qualification gates.
- `Boat/tests/verify_safety_cutoff_netlist_r10.py`: KiCad XML exact device pin-net checks.
- `.github/workflows/boat-kicad-safety-r10.yml`: real KiCad CLI native schematic ERC/netlist validation.

**The R10 schematic is NOT a complete physical safety system.** Zero ERC on this standalone sheet, if achieved, only means KiCad caught no connection-class errors in this isolated candidate.


## R10 safety improvement: independent third STOP witness (still unqualified)
Initial R10 review detected a dangerous auto-rearm path if the J3 second-pole STOP_OK sensor stayed falsely HIGH while S1 interrupted Q2 source ground. Native R10 now includes **S2, a third mechanically linked normally-closed contact**, 10k pullup R5 and U5 SN74HC14 Schmitt inverter. S2 closed pulls STOP_SENSE_N low and inverter drives STOP_CONTACT_CLOSED_OK high; opening S2 forces a LOW input into the extra U3 AND stage. This asynchronously clears U4's SN74HC74 and prevents automatic reenergization after STOP release, even with J3 STOP_OK stuck HIGH. The safety chain is now 10 J3 inputs **plus the independent S2 contact witness**. S1 remains purely in the Q2 source-return path, and S2 sense is electrically separate; connecting STOP_SENSE_N to Q2 source would defeat the physical STOP.

A *single actual triple-pole mechanically linked STOP switch* is necessary (S1 Q2 source cutoff, an independently conditioned J3 STOP_OK, and S2 separately sensed NC). Hardware mechanical linkage, test of contact synchronization, contact weld behavior, SN74HC14 thresholds and input wiring all remain unsigned. It is a candidate only. Reference: https://www.ti.com/product/SN74HC14 .
