#!/usr/bin/env python3
"""Boat 3S pack / 20A BMS peak-current gate.

No guessed motor-stall, coil, regulator or fuse values; fail on missing data.
The output is a conservative *budget*, not a safety certification.
"""
from dataclasses import dataclass
import math
from typing import Optional

@dataclass(frozen=True)
class LoadInputs:
    battery_min_v: float
    bms_continuous_a: float
    left_motor_stall_a: Optional[float]
    right_motor_stall_a: Optional[float]
    coil_max_pulse_a: Optional[float]
    logic_5v_peak_a: Optional[float]
    logic_buck_efficiency: Optional[float]
    reserve_fraction: float=0.2

def calculate(l: LoadInputs) -> dict:
    known=(l.left_motor_stall_a,l.right_motor_stall_a,l.coil_max_pulse_a,
           l.logic_5v_peak_a,l.logic_buck_efficiency)
    if any(v is None for v in known):
        raise ValueError("MEASUREMENT REQUIRED: motor stall, coil peak, 5V load and converter efficiency")
    if any(not math.isfinite(float(v)) for v in known) or not all(float(v)>0 for v in known):
        raise ValueError("invalid or zero load")
    if not all(math.isfinite(x) and x>0 for x in
               (l.battery_min_v,l.bms_continuous_a)) or l.battery_min_v<6:
        raise ValueError("invalid battery/BMS parameters")
    if not 0<l.logic_buck_efficiency<1 or not 0<=l.reserve_fraction<0.8:
        raise ValueError("invalid efficiency or reserve")
    logic_input_a=5*l.logic_5v_peak_a/(l.battery_min_v*l.logic_buck_efficiency)
    # Software forbids motor + release overlap; HARDWARE interlock must also
    # prevent this. Do not sum motor and coil as simultaneous steady loads.
    motors_input=l.left_motor_stall_a+l.right_motor_stall_a+logic_input_a
    release_input=l.coil_max_pulse_a+logic_input_a
    usable=l.bms_continuous_a*(1-l.reserve_fraction)
    return {
        "motors_peak_a":round(motors_input,3),
        "release_peak_a":round(release_input,3),
        "logic_input_a":round(logic_input_a,3),
        "budget_usable_a":round(usable,3),
        "motors_within_budget":motors_input<=usable,
        "release_within_budget":release_input<=usable,
        "sufficient":max(motors_input,release_input)<=usable,
        "caveat":"BMS 20A label is not verified peak-trip curve. Full fuse, stall, transient, heat and switching SOA tests still required."
    }
if __name__=="__main__":
    print("INCOMPLETE: no measured 550 motor stall or solenoid currents. No current/battery/fuse freeze.")
