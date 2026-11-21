#!/usr/bin/env python3
"""R17 COTS-only interlock candidate/harness verification.

Checks repository consistency and preserves physical test blockers.
Never treats vendor nameplates, GitHub CI or simulated interlocks as
physical proof of actuator safety.
"""
from pathlib import Path
import csv

ROOT=Path(__file__).resolve().parents[2]
CANDIDATES=ROOT/"docs/MODULE_ONLY_CANDIDATES_R17.csv"
HANDOFF=ROOT/"Boat/docs/BOAT_R17_MODULE_SAFETY_HANDOFF.csv"
GATES=ROOT/"Boat/docs/BOAT_R17_RELEASE_GATES.csv"
DESIGN=ROOT/"Boat/docs/BOAT_PREMADE_SAFETY_STACK_R17.md"
BOM=ROOT/"docs/MODULE_ONLY_BOM_R15.csv"
R16=ROOT/"Boat/docs/BOAT_EKM014_SELECTED_R16.md"

def rows(p):
    with p.open(encoding="utf-8",newline="") as f:
        return list(csv.DictReader(f))

def check():
    design=DESIGN.read_text(encoding="utf-8")
    assert "NO ACTUATOR POWER" in design
    assert "no custom" in design.lower() or "custom electronics PCBs" in design
    assert "42042237" in design and "SW60C" in design
    assert "SW60P" in design and "NOT AVAILABLE (X)" in design
    assert "permanent X1–X2 bridge allows automatic reset" in design
    assert "3 A at 24 V DC resistive outputs" in design
    assert "no-repeat" in design.lower() and "onehot" in design.lower()
    assert "MONITORED" in design.upper()
    assert "R9/R10/R11/R12" not in design or "custom" in design.lower()

    parts=rows(CANDIDATES)
    assert len(parts)==11,len(parts)
    by_role={r["Role"]:r for r in parts}
    assert len(by_role)==len(parts)
    assert by_role["EKM014_4_COILS"]["Model_or_ID"]=="EKM014 41024394"
    assert by_role["EKM014_4_COILS"]["Disposition"]=="OWNER_SELECTED_NOT_COMMISSIONED"
    assert by_role["ESTOP_MONITOR"]["Model_or_ID"]=="NST-3.2 42042237"
    assert by_role["ESTOP_MONITOR"]["Disposition"]=="CANDIDATE_NOT_SELECTED"
    assert by_role["DC_MAIN_CUTOFF_SEALED_BUT_NO_AUX"]["Disposition"]=="NOT_PREFERRED"
    assert by_role["DC_MAIN_CUTOFF_MONITORED"]["Model_or_ID"].startswith("SW60C")
    assert by_role["FOUR_CHANNEL_PULSE_FUNCTIONAL"]["Disposition"]=="CANDIDATE_NOT_SAFETY_QUALIFIED"
    assert by_role["INDEPENDENT_NO_REPEAT_ONEHOT"]["Disposition"]=="REQUIRED_UNRESOLVED"
    assert by_role["INDEPENDENT_POR"]["Disposition"]=="REQUIRED_UNRESOLVED"
    assert by_role["INDEPENDENT_CURRENT_ZERO"]["Disposition"]=="REQUIRED_UNRESOLVED"
    assert all(r["Source"] or r["Disposition"]=="REQUIRED_UNRESOLVED" for r in parts)

    gates=rows(GATES)
    assert len(gates)==21,len(gates)
    assert {row["ID"] for row in gates}=={f"R17-G{i:02d}" for i in range(1,22)}
    assert {g["Status"] for g in gates}=={"NOT_VERIFIED","NOT_APPLICABLE"}
    assert sum(g["Status"]=="NOT_VERIFIED" for g in gates)==20
    assert all(g["Acceptance_authority"] in ("HUMAN_MEASUREMENT","MANUFACTURER","REQUIREMENT") for g in gates)
    assert not any(g["Status"] in ("PASS","APPROVED","COMMISSIONED") for g in gates)

    handoff=rows(HANDOFF)
    assert len(handoff)==18,len(handoff)
    assert all("NOT_WIRED" in r["State"] or r["State"] in ("UNSELECTED","UNQUALIFIED_DO_NOT_CONNECT")
               for r in handoff)
    coil_input_rows=[r for r in handoff if r["Stage"].startswith("RELEASE_")]
    assert len(coil_input_rows)==4
    for n,row in enumerate(coil_input_rows,1):
        assert row["Destination"]=="Electrokit EKM014"
        assert row["Input_or_output"]==f"IN{n}"
        assert row["State"]=="UNSELECTED"
        assert "independent" in row["Source"].lower()
    for row in handoff:
        # A safe-stop supervisor must NOT be wired into main DC load terminals.
        assert not (row["Source"].startswith("Duelco") and row["Destination"] in ("Cytron MDD20A","Electrokit EKM014"))
        # EKM014 user-chosen 4-channel board may never be an automatic live-load.
        assert "COMMISSIONED" not in row["State"]

    bom=rows(BOM)
    selected=[p for p in bom if p["Side"]=="Boat" and p["Function"]=="Four solenoid low-side power"]
    assert len(selected)==1 and selected[0]["Vendor"]=="Electrokit"
    assert selected[0]["Part_Number"]=="EKM014"
    assert selected[0]["State"]=="OWNER_SELECTED_NOT_INSTALLED_NOT_QUALIFIED"
    assert "UNCOMMISSIONED" in R16.read_text(encoding="utf-8")
    print("R17 MODULE SAFETY STACK CONTRACT PASS: 11 candidates, 18 unconnected interface rows, 21 release gates")
    print("ALL 20 applicable physical gates NOT VERIFIED; no hardware output commissioned")

if __name__=="__main__":
    check()
