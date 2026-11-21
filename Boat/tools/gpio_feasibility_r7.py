#!/usr/bin/env python3
"""R7 conservative GPIO budget. Candidate pins are not an authorized pinout.

Heltec V1.1 published map + community V1.2 board_pins.h cross-check.
Zero pins are considered verified for actual V1.2 before physical audit.
"""
from dataclasses import dataclass
from typing import FrozenSet, Sequence
import csv
from pathlib import Path

# UART0 reclaim means no ordinary serial console, with verified USB CDC.
# GPIO15/16 oscillator availability and GPIO37 flash/control must be proven.
CANDIDATES = frozenset((4, 5, 6, 7, 15, 16, 17, 18, 37, 43, 44))
CONTESTED = frozenset((15, 16, 18, 37, 43, 44))
STRICT_CANDIDATES = CANDIDATES - CONTESTED
# The native SX1262 SPI data pins 9/10/11 are already occupied.
# Internal SX1262 uses prewired board SPI: no additional carrier UART.
CONSUMERS = {
 "nrf_ce":1,"nrf_csn":1,"i2c_sda":1,"i2c_scl":1,
 "mdd_pwm_left":1,"mdd_pwm_right":1,
 "mdd_dir_left":1,"mdd_dir_right":1,
 "external_watchdog_wdi":1,"independent_gate_permit":1,
}
EXPANDER_ELIGIBLE = frozenset(("coil_selection","slow_feedback"))
REQUIRED_NATIVE = frozenset(("nrf_ce","nrf_csn","mdd_pwm_left",
 "mdd_pwm_right","mdd_dir_left","mdd_dir_right",
 "external_watchdog_wdi","independent_gate_permit"))
def required_native_count(**changes: bool)->int:
 count=len(CONSUMERS)
 if changes.get("external_e22",False):count+=2
 if changes.get("direction_expander",False):count-=2 # NOT APPROVED
 return count
def budget(pins: FrozenSet[int]=CANDIDATES,**changes: bool)->dict:
 demand=required_native_count(**changes)
 return {"candidate_count":len(pins),"demand":demand,
  "shortfall":max(0,demand-len(pins)),
  "feasible_on_count_alone":len(pins)>=demand}
def validate_assignment(mapping: dict, permitted: FrozenSet[int],
                        committed: bool = False) -> bool:
    if not committed or set(mapping) != set(CONSUMERS):
        return False
    values = list(mapping.values())
    if any(type(p) is not int or p not in permitted for p in values):
        return False
    return len(set(values)) == len(values)
def load_rows(path: Path) -> list:
    with path.open(newline="",encoding="utf-8") as f:
        rows=list(csv.DictReader(f))
    if len({r["GPIO"] for r in rows}) != len(rows):
        raise ValueError("duplicate GPIO candidate")
    return rows

if __name__ == "__main__":
    import json
    print(json.dumps({
        "status":"BLOCKED: V1.2 exact board and external GPIO budget unverified",
        "candidate_pins":sorted(CANDIDATES),
        "contested_pins":sorted(CONTESTED),
        "strict_baseline":budget(STRICT_CANDIDATES),
        "optimistic_baseline":budget(),
        "retired_external_e22_baseline":budget(external_e22=True),
        "onboard_sx1262_baseline":budget(),
        "with_unapproved_direction_expander":budget(direction_expander=True),
        "implementation":"No pins enabled; backup PHY unmatched; sonar remains on hold",
    },indent=2))
