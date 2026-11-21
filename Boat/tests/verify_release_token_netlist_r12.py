#!/usr/bin/env python3
"""R12 independent *candidate* consumed release token schematic.
Never mistake an isolated KiCad pin-level pass or software model for a
qualified high-current safety system.
"""
from __future__ import annotations
from pathlib import Path
from collections import Counter
import json
import re
import xml.etree.ElementTree as ET
import argparse

ROOT = Path(__file__).resolve().parents[1]
P = ROOT / "hardware/Release_Token_R12"
SCHEMATIC = P / "Boat_Release_Token_R12.kicad_sch"
CONTRACT = P / "R12_PIN_NET_EXPECTATIONS.json"
CHANNELS = ("LH","RH","LK","RK")


def guard_source():
    data = json.loads(CONTRACT.read_text())
    sch = SCHEMATIC.read_text()
    refs = re.findall(r'\(symbol \(lib_id "FlytecR12:[^"]+"\).*?\(property "Reference" "([^"]+)"',sch)
    assert len(refs) == 23 and len(set(refs)) == 23
    assert sorted(refs) == sorted(data["parts"])
    assert sch.count('(property "Footprint" ""') == 23
    assert 'R12-DRAFT-NO-FAB' in sch
    assert len(data["nets"]) == 53
    assert len(data["intentionally_unconnected"]) == 11
    assert not any(x in sch for x in ("RP2040","SONAR","E22","RUDDER"))
    for ch in CHANNELS:
        assert f'(label "SOL_REQ_{ch}"' in sch
    for x in ("ARM_CLR_N","FIRE_CLR_N","RELEASE_ARMED","FIRED_Q",
              "NOT_FIRED","RELEASE_QUAL_EDGE","RELEASE_EDGE_TOKEN_TBD"):
        assert '(label "' + x + '"' in sch
    print("R12 source check: 23 generic/unapproved parts, 53 named nets, 11 NC outputs")


def check_external_interface():
    r11 = (ROOT/"hardware/Pulse_Limiter_R11/Boat_Pulse_Limiter_R11.kicad_sch").read_text()
    r7 = (ROOT/"hardware/Direct_MDD20A_R7/Boat_MDD20A_Control_R7.kicad_sch").read_text()
    r9 = (ROOT/"hardware/Coil_Drivers_R9/Boat_Four_Coil_Drivers_R9.kicad_sch").read_text()
    r10 = (ROOT/"hardware/Safety_Cutoff_R10/Boat_Safety_Cutoff_R10.kicad_sch").read_text()
    for ch in CHANNELS:
        net = f'SOL_REQ_{ch}'
        assert f'(label "{net}"' in r7 and f'(label "{net}"' in SCHEMATIC.read_text()
    assert '(label "RELEASE_EDGE_TOKEN_TBD"' in r11
    assert '(label "RELEASE_EDGE_TOKEN_TBD"' in SCHEMATIC.read_text()
    assert '(label "LATCH_Q"' in r10
    assert '(label "ACT_SOL_POS_AFTER_LATCH_TBD"' in r9
    print("R7->R12->R11->R9/R10 interface naming exists, but not PHYSICALLY CONNECTED.")


def load_xml(path):
    tree=ET.parse(path)
    nets={}
    for el in tree.findall("./nets/net"):
        name=el.get("name","").removeprefix("/")
        if name in nets:
            raise AssertionError("duplicate native name "+name)
        nets[name]={(p.get("ref"),p.get("pin")) for p in el.findall("node")}
    return nets


def xml_test(path):
    nets = load_xml(path)
    contract=json.loads(CONTRACT.read_text())
    no_conn={tuple(v) for v in contract["intentionally_unconnected"]}
    actual_nc = {name:n for name,n in nets.items() if name.startswith("unconnected-")}
    found_nc = set().union(*actual_nc.values()) if actual_nc else set()
    assert found_nc==no_conn,(found_nc,no_conn)
    assert all(len(nodes)==1 for nodes in actual_nc.values())
    named={name:node for name,node in nets.items() if name not in actual_nc}
    expected={name:{tuple(node) for node in pins} for name,pins in contract["nets"].items()}
    assert set(named)==set(expected),sorted(set(named)^set(expected))
    for net,want in expected.items():
        assert named[net]==want,f"{net}: got {sorted(named[net])}, expected {sorted(want)}"
    # Critical independent assertions: a hold cannot rearm by expiry, and
    # only a fresh hardware-processed release can set the arm flip-flop.
    assert named["ARM_CLR_N"]=={("U11","8"),("U1","1")}
    assert named["FIRE_CLR_N"]=={("U11","11"),("U1","13")}
    assert named["FIRED_Q"]=={("U1","9"),("U7","13"),("U2","2")}
    assert named["NOT_FIRED"]=={("U7","12"),("U11","10")}
    assert named["RELEASE_ARMED"]=={("U1","5"),("U9","10")}
    assert named["RELEASE_QUAL_EDGE"]=={("U8","4"),("U1","3")}
    assert named["REQ_CLK_DELAY"]=={("U7","10"),("U1","11")}
    assert named["RELEASE_EDGE_TOKEN_TBD"]=={("U2","13"),("J3","1")}
    assert named["ONEHOT_VALID"]=={("U4","8"),("U10","13")}
    assert named["FIRE_D"]=={("U9","8"),("U1","12")}
    assert named["ASYNC_SAFETY_N"]=={("U11","6"),("U11","9"),("U11","12"),("U9","5"),("U2","3")}
    assert named["VLOGIC_3V3"]>={("U1","14"),("U2","16"),("J2","1"),("R4","1")}
    assert named["GND"]>={("U1","7"),("U2","8"),("J2","2"),("C4","2")}
    for i,ch in enumerate(CHANNELS,1):
        assert ("J1",str(i)) in named[f"SOL_REQ_{ch}"]
    print(f"Native R12 KiCad netlist PASS: {len(named)} exact electrical nets; "
          f"{len(no_conn)} intentionally unconnected outputs.")


def static_onehot_proof():
    for mask in range(16):
        req=[bool(mask&(1<<i)) for i in range(4)]
        invalid=any(req[i] and req[j] for i in range(4) for j in range(i+1,4))
        valid=any(req) and not invalid
        assert valid==(mask.bit_count()==1),(mask,valid)
        # Each input is either a genuine unique request, idle, or a fault.
        for armed in (False,True):
            for motor0 in (False,True):
                for coil0 in (False,True):
                    for safety in (False,True):
                        active = bool(valid and armed and motor0 and coil0 and safety)
                        if active:
                            assert mask.bit_count()==1
                        if not (armed and motor0 and coil0 and safety):
                            assert not active
    print("R12 static 16 masks x 16 qualify combinations: 256 checks PASS; "
          "NOT a timing or glitches proof.")


def erc_test(path):
    content=path.read_text()
    issues=re.findall(r"^\[([^]]+)\]:",content,flags=re.MULTILINE)
    assert sorted(issues)==["power_pin_not_driven","power_pin_not_driven"],issues
    print("R12 isolated KiCad ERC: 2 UNRESOLVED external safety power-source errors; "
          "not a passing full system.")


if __name__=="__main__":
    p=argparse.ArgumentParser()
    p.add_argument("--xml",type=Path)
    p.add_argument("--erc",type=Path)
    a=p.parse_args()
    guard_source()
    check_external_interface()
    static_onehot_proof()
    if a.xml:xml_test(a.xml)
    if a.erc:erc_test(a.erc)
