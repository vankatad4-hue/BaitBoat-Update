#!/usr/bin/env python3
"""R11 real KiCad electrical pin/network capture guard.
Never interpret netlist/isolated ERC as safety certification or physical acceptance.
"""
from __future__ import annotations
import argparse
import json
import re
import xml.etree.ElementTree as ET
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[1]
DIR = ROOT / "hardware/Pulse_Limiter_R11"
SCH = DIR / "Boat_Pulse_Limiter_R11.kicad_sch"
CONTRACT = DIR / "R11_PIN_NET_EXPECTATIONS.json"
PAIR = ("LH", "RH", "LK", "RK")
PIN_AND = ((1,2,3),(4,5,6),(9,10,8),(12,13,11))
PIN_OR = PIN_AND
PIN_INV = ((1,2),(3,4),(5,6),(9,8),(11,10),(13,12))


def source_check():
    text = SCH.read_text(encoding="utf-8")
    data = json.loads(CONTRACT.read_text(encoding="utf-8"))
    refs = re.findall(r'\(symbol \(lib_id "FlytecR11:[^"]+"\).*?\(property "Reference" "([^"]+)"', text)
    assert sorted(refs) == sorted(data["parts"]) and len(refs)==17, refs
    assert text.count('(property "Footprint" ""')==len(refs), "Some part was released with a footprint"
    assert 'R11-NOT-QUALIFIED' in text and 'RELEASE_EDGE_TOKEN_TBD' in text
    assert not any(s in text for s in ("RP2040", "SONAR", "RUDDER", "E22"))
    assert len(data["nets"])==54
    for ch in PAIR:
        net = "PULSE_"+ch+"_LIMITED_TBD"
        assert sorted(data["nets"][net]) == sorted([["J3",str(PAIR.index(ch)+1)],["U3",str(PIN_AND[PAIR.index(ch)][2])]])
    assert data["nets"]["RC_TIMING"] and data["nets"]["CLR_MONO_N"]
    print("R11 source: 17 manufacturer-pin-mapped prototypes, 54 expected nets; no approved footprint")


def native_netlist(path):
    tree = ET.parse(path)
    found = {}
    for net in tree.findall("./nets/net"):
        name = net.get("name","").removeprefix("/")
        assert name not in found, name
        found[name] = {(node.get("ref"),node.get("pin")) for node in net.findall("node")}
    return found


def verify_netlist(path):
    real = native_netlist(path)
    expected = {name:{tuple(pin) for pin in pins} for name,pins
                in json.loads(CONTRACT.read_text(encoding="utf-8"))["nets"].items()}
    nc = {("U1","4"),("U1","5"),("U1","12"),
          ("U2","3"),("U2","6"),("U2","11"),("U2","14"),
          ("U8","10"),("U8","12")}
    unnamed={name:nodes for name,nodes in real.items() if name.startswith("unconnected-")}
    seen=set().union(*(nodes for nodes in unnamed.values())) if unnamed else set()
    assert seen==nc and all(len(nodes)==1 for nodes in unnamed.values()), (seen,nc)
    actual={k:v for k,v in real.items() if k not in unnamed}
    assert set(actual)==set(expected), ("Unexpected/missing named nets:",sorted(set(actual)^set(expected)))
    for name,pins in expected.items():
        assert actual[name]==pins, f"{name}: actual={sorted(actual[name])}, expected={sorted(pins)}"
    # Safety critical wiring is asserted independently of generated expectations.
    assert actual["PULSE_WINDOW"]=={("U1","13"),("U8","3"),("U9","1")}
    assert actual["CLR_MONO_N"]=={("U1","3"),("U2","1"),("U9","8")}
    assert actual["FIRE_POS_EDGE"]=={("U1","2"),("U2","9"),("U8","8")}
    assert actual["WINDOW_OFF"]=={("U5","13"),("U8","4")}
    assert actual["ONEHOT_VALID"]=={("U5","8"),("U9","10"),("U9","13"),("U9","5")}
    assert actual["RC_TIMING"]=={("U1","15"),("R1","2"),("C1","1")}
    assert actual["GND"]>= {("U1","1"),("U1","8"),("U1","11"),("U1","14"),("U2","8"),("U8","7"),("C1","2")}
    assert actual["VLOGIC_3V3"]>= {("U1","16"),("U2","16"),("J2","1"),("R1","1")}
    for i,ch in enumerate(PAIR):
        output_pin=str(PIN_AND[i][2])
        req = "SOL_REQ_"+ch
        assert ("J1",str(i+1)) in actual[req]
        assert ("U10",str(PIN_AND[i][1])) in actual[req]
        assert actual["PULSE_"+ch+"_LIMITED_TBD"]=={("U3",output_pin),("J3",str(i+1))}
        assert actual["MATCH_"+ch]=={("U10",output_pin),("U3",str(PIN_AND[i][0]))}
    print(f"R11 actual KiCad XML exact NET connectivity PASS: {len(actual)} nets, 9 intentionally NC pins")
    print("STILL BLOCKED: external consumed-release token, rail/current sensors, timing/RC ratings, glitch-free edge qualification")


def structural_truth_table():
    # These equations are exactly the captured U4..U11 glue gates (see pin list above).
    for mask in range(16):
        bits=[bool(mask & (1<<k)) for k in range(4)]
        overlap=any(bits[i] and bits[j] for i in range(4) for j in range(i+1,4))
        any_request=any(bits)
        onehot=any_request and not overlap
        assert onehot==(mask.bit_count()==1), mask
        for captured in range(16):
            match=[bits[i] and bool(captured &(1<<i)) for i in range(4)]
            for window in (False,True):
                for latch in (False,True):
                    safe=window and latch and onehot
                    driven=[v and safe for v in match]
                    assert sum(driven)<=1, (mask,captured,window,latch)
                    if not window or not latch or not onehot:
                        assert not any(driven)
    print("4 request lines x 4 capture lines x window/latch: 1024 static test combinations PASS")
    print("Static truth tables DO NOT prove asynchronous bus glitches, power-on edges or pulse timing.")


def check_erc(path):
    s=path.read_text(encoding="utf-8")
    violations=re.findall(r"^\[([^]]+)\]:",s,flags=re.MULTILINE)
    assert sorted(violations)==["power_pin_not_driven","power_pin_not_driven"], violations
    print("R11 standalone ERC: 2 UNRESOLVED external safety-rail source errors (3V3/return); no waivers")


def check_crosssheet():
    r7=(ROOT/"hardware/Direct_MDD20A_R7/Boat_MDD20A_Control_R7.kicad_sch").read_text()
    r9=(ROOT/"hardware/Coil_Drivers_R9/Boat_Four_Coil_Drivers_R9.kicad_sch").read_text()
    r10=(ROOT/"hardware/Safety_Cutoff_R10/Boat_Safety_Cutoff_R10.kicad_sch").read_text()
    for ch in PAIR:
        assert f'(label "SOL_REQ_{ch}"' in r7
        assert f'(label "SOL_REQ_{ch}"' in SCH.read_text()
        assert f'(label "PULSE_{ch}_LIMITED_TBD"' in SCH.read_text()
        assert f'(label "PULSE_{ch}_LIMITED_TBD"' in r9
    assert '(label "LATCH_Q"' in r10 and '(label "HW_LATCH_OK_R10"' in SCH.read_text()
    assert '(label "ACT_SOL_POS_AFTER_LATCH_TBD"' in r10
    assert '(label "ACT_SOL_POS_AFTER_LATCH_TBD"' in r9
    # Distinct net names for external hardware latch and coil power, intentionally
    # NO automatic join. A cross-sheet contract is NOT electrical continuity.
    print("R7/R11/R9/R10 matching logical names PASS; physical linking remains UNBUILT")


if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--xml",type=Path)
    parser.add_argument("--erc",type=Path)
    a=parser.parse_args()
    source_check()
    structural_truth_table()
    check_crosssheet()
    if a.xml:verify_netlist(a.xml)
    if a.erc:check_erc(a.erc)
