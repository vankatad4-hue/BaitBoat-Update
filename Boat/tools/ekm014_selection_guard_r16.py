#!/usr/bin/env python3
"""R16 enforce owner-selected Electrokit EKM014 and hardware-uncommissioned state.
A checked selection is NOT proof of coil safety nor permission to energize.
"""
import csv
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
def table(path):
    with (ROOT/path).open(newline="",encoding="utf-8") as f:
        return list(csv.DictReader(f))
def verify():
    bom=table("docs/MODULE_ONLY_BOM_R15.csv")
    matches=[r for r in bom if r["Side"]=="Boat" and r["Function"]=="Four solenoid low-side power"]
    assert len(matches)==1,matches
    part=matches[0]
    assert part["Vendor"]=="Electrokit" and part["Part_Number"]=="EKM014"
    assert part["Quantity"]=="1" and part["State"]=="OWNER_SELECTED_NOT_INSTALLED_NOT_QUALIFIED",part
    assert part["Manufacturer_Reference"]=="https://docs.electrokit.com/en/modules/EKM014/"
    contract=(ROOT/"Boat/docs/BOAT_EKM014_SELECTED_R16.md").read_text(encoding="utf-8")
    assert "41024394" in contract and "UNCOMMISSIONED" in contract
    assert "10 A slow-blow" in contract and "VH >= VL" in contract
    assert "not" in contract.lower() and "no custom" in contract.lower()
    assert "independent" in contract.lower()
    wiring=table("Boat/docs/BOAT_EKM014_PIN_WIRING_R16.csv")
    assert len(wiring)==12,len(wiring)
    inputs={r["EKM014 terminal or pin"]:r for r in wiring if r["Area"]=="Control"}
    outputs={r["EKM014 terminal or pin"]:r for r in wiring if r["Area"]=="Output"}
    assert set(inputs)=={"IN1","IN2","IN3","IN4"}
    assert set(outputs)=={"CH1 + and -","CH2 + and -","CH3 + and -","CH4 + and -"}
    for i,tag in enumerate(("LH","RH","LK","RK"),start=1):
        assert inputs[f"IN{i}"]["Logical channel"] == f"{i-1} {tag}"
        assert outputs[f"CH{i} + and -"]["Logical channel"] == f"{i-1} {tag}"
        assert "NOT_WIRED" in inputs[f"IN{i}"]["Module status"]
        assert inputs[f"IN{i}"]["From or destination"].find("NOT SELECTED")!=-1
    assert any(r["EKM014 terminal or pin"]=="VL 2.54mm header" and "5V" in r["Signal"]
               for r in wiring)
    assert any(r["EKM014 terminal or pin"]=="VH supply screw terminal"
               for r in wiring)
    assert "OWNER SELECTED" in (ROOT/"Boat/docs/BOAT_MODULE_ONLY_IMPLEMENTATION_R15.md").read_text(encoding="utf-8")
    assert "R16" in (ROOT/"docs/CONTINUATION_CHECKPOINT_VER001.md").read_text(encoding="utf-8")
    for path in ("Boat/docs/BOAT_EKM014_ACCEPTANCE_R16.md","Boat/docs/BOAT_EKM014_SELECTED_R16.md"):
        assert "NOT" in (ROOT/path).read_text(encoding="utf-8")
    assert "R15" in (ROOT/"Boat/hardware/README.md").read_text(encoding="utf-8")
    print("R16 PASS: owner-selected EKM014 part 41024394, 1 board, 4 exact channels,")
    print("4 matching output pairs; no physical supply, safety interlock or GPIO commissioned.")
if __name__=="__main__":verify()
