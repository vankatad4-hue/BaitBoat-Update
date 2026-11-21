#!/usr/bin/env python3
"""Check cross-sheet interface names; these are still separate KiCad projects.

A matched label is NOT a wire, a connector footprint, or an ERC-certified
hierarchy. Never auto-connect an unverified high-current or safety interface.
"""
from pathlib import Path
import csv
import re

ROOT=Path(__file__).resolve().parents[1]
S={
    "R7":(ROOT/"hardware/Direct_MDD20A_R7/Boat_MDD20A_Control_R7.kicad_sch").read_text(),
    "R9":(ROOT/"hardware/Coil_Drivers_R9/Boat_Four_Coil_Drivers_R9.kicad_sch").read_text(),
    "R10":(ROOT/"hardware/Safety_Cutoff_R10/Boat_Safety_Cutoff_R10.kicad_sch").read_text(),
}

def labels(text):
    return set(re.findall(r'\(label "([^"]+)"',text))

def check():
    all_labels={key:labels(sheet) for key,sheet in S.items()}
    matches=[
        ("R7","WDO_N","R10","WDO_N"),
        ("R7","ACT_PERMIT","R10","MCU_PERMIT"),
        ("R10","ACT_SOL_POS_AFTER_LATCH_TBD","R9","ACT_SOL_POS_AFTER_LATCH_TBD"),
        ("R10","PACK_RETURN","R9","ACT_RETURN"),
    ]
    for sa,na,sb,nb in matches:
        assert na in all_labels[sa],(sa,na)
        assert nb in all_labels[sb],(sb,nb)
    for ch in ("LH","RH","LK","RK"):
        assert "SOL_REQ_"+ch in all_labels["R7"]
        assert "PULSE_"+ch+"_LIMITED_TBD" in all_labels["R9"]
        assert "PULSE_"+ch+"_LIMITED_TBD" not in all_labels["R10"]
    # R7 has a WDO pull-up resistor already; R10 R1 must be DNP
    # if R7's R1 is populated. Parallel copies are not a design decision.
    assert '(label "WDO_N"' in S["R7"]
    assert '(property "Value" "10k"' in S["R7"]
    assert '(property "Value" "DNP_IF_R7_WDO_PULLUP_FITTED"' in S["R10"]
    matrix=ROOT/"docs/BOAT_R7_R9_R10_INTERFACE_CONTRACT.csv"
    rows=list(csv.DictReader(matrix.open(newline="",encoding="utf-8")))
    assert len(rows)>=10
    assert all(r["Status"]=="UNVERIFIED_HARNESS_OR_HIERARCHY" for r in rows)
    assert "RP2040" not in S["R10"] and "SONAR" not in S["R10"]
    print("Standalone R7/R9/R10 labels and DNP watchdog pullup interface checks passed")
    print("NOT PASSED: electrical hierarchy, current return layout, actual connectors, full ERC")


if __name__=="__main__":check()
