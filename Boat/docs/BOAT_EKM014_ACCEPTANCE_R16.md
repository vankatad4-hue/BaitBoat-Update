# EKM014 — R16 real-module acceptance checklist (SELECTED; NOT COMMISSIONED)
- [ ] Exact vendor module **EKM014 41024394** physically received and label/photo documented; supplied 2.54mm header and five 3.5mm terminal pairs soldered/retained per manufacturer instructions.
- [ ] Confirm board hole centers from actual module: technical docs 27x25 mm vs listing 41x25 mm; board outline 32x30 mm. Adjust only 3D printed non-electronic brackets, never custom PCB.
- [ ] Confirm correct VIN/return path: VH supplied after master switch, rated fuse, independent STOP contactor; power ground uses rated return cables; no actuator current on Heltec return.
- [ ] Verify dedicated regulated **5V VL** voltage with 3S 9–12.6V, power-on, brownout and battery disconnect; **VL must never exceed 12V**, and VH>=VL.
- [ ] Verify all IN1..IN4 low/off before and during Heltec boot, broken signal cable, timer power loss, unplugged interlock, 3S brownout, and stuck-high MCU.
- [ ] Real independent commercial hardware one-hot and maximum pulse timer with consumed/rearm behavior available and independently tested; GPIO-only enforcement DOES NOT qualify.
- [ ] Verify no more than one input/output may be energized at any instant, including two simultaneous requests and timing skew/glitches.
- [ ] Verify no-repeat for an indefinitely held HIGH command, STOP/brownout fail-OFF with deliberate manual rearm.
- [ ] Bench **unloaded** driver logic power and individual input/LED behavior first on current-limited bench supply, without connecting coils or motors.
- [ ] Measure separately each original FW0630-009 and FW0630-001 resistance/current/inductive flyback, from minimum loaded 3S through fully charged 12.6V, then choose allowable maximum pulse time and thermal/relay limits.
- [ ] Qualify shared board 10A slow fuse and external per-branch fuse/harness ratings; test under physical one-coil-at-a-time operation (20A BMS not a safety fuse).
- [ ] Check Schottky flyback suppression and actual solenoid release delay; verify coil de-energizes and zero-current feedback before any rearm.
- [ ] Test independent DC-actuator cutoff under real rated dummy motor/solenoid loads including welded/failed input, latch loss, battery drop/recovery, watchdog failure and disconnected stop.
- [ ] Record real GPIO/header physical pin map, manufacturer board revision, instrumented logs, oscilloscope pulse max and hardware commissioning signature.
- [ ] Only after independent safety review connect actual solenoids; no physical tests can be claimed by ChatGPT without actual hardware and measurements.

Test result: **NOT PERFORMED.** No live actuator authorization. Boat VER014 and Remote VER028 unchanged. Sonar not selected.
