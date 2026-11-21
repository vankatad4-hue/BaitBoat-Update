#!/usr/bin/env python3
"""R13 pin-accurate verification of ready-made RECOM R-78E3.3-0.5 supply.
Not physical safety certification or full R7-R13 ERC sign-off.
"""
from __future__ import annotations
import argparse
import json
import re
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
P = ROOT / "hardware/Safety_Power_R13"
SCH = P / "Boat_Safety_Power_R13.kicad_sch"
SPEC = P / "R13_PIN_NET_EXPECTATIONS.json"
REFS = {"J1","J2","J3","U1","F1","C1","C2","C3","R1"}


def guard_source():
    s = SCH.read_text(encoding="utf-8")
    contract = json.loads(SPEC.read_text(encoding="utf-8"))
    refs = re.findall(r'\(symbol \(lib_id "FlytecR13:[^"]+"\).*?\(property "Reference" "([^"]+)"',s)
    assert set(refs) == REFS and len(refs)==len(REFS)
    assert s.count('(property "Footprint" ""') == len(REFS), "No approved footprints"
    assert "R13-UNQUALIFIED" in s and 'RECOM_R-78E3.3-0.5_PIN1_VIN_PIN2_GND_PIN3_VOUT' in s
    assert contract["manufacturer"]=="RECOM" and contract["part_number"]=="R-78E3.3-0.5"
    assert contract["source_specs"]["input_min_v"]==6
    assert contract["source_specs"]["input_max_v"]==28
    assert contract["source_specs"]["output_v"]==3.3
    assert contract["source_specs"]["max_output_ma"]==500
    assert contract["source_specs"]["maximum_capacitive_load_uF"]==220
    assert sorted(contract["parts"])==sorted(REFS)
    assert len(contract["nets"])==4
    assert not any(token in s for token in ("RP2040","SONAR","RUDDER","Boat E22"))
    print("R13 source PASS: 9 unqualified parts; verified vendor fixed pinout and 6-28V/3.3V/0.5A")


def verify_xml(path):
    doc=ET.parse(path)
    actual={}
    for net in doc.findall("./nets/net"):
        name=net.get("name","").removeprefix("/")
        assert name not in actual
        actual[name]={(node.get("ref"),node.get("pin")) for node in net.findall("node")}
    expected={name:{tuple(item) for item in pins}
              for name,pins in json.loads(SPEC.read_text())["nets"].items()}
    assert set(actual)==set(expected), ("Unexpected/missing nets",sorted(set(actual)^set(expected)))
    for name,expected_pins in expected.items():
        assert actual[name]==expected_pins, (name,sorted(actual[name]),sorted(expected_pins))
    # Independent manufacturer pin-number and separated power-interface assertions:
    assert actual["PACK_SWITCHED_3S_TBD"]=={("J1","1"),("F1","1")}
    assert actual["PACK_SWITCHED_LOGIC_FUSED"]=={("F1","2"),("U1","1"),("C1","1")}
    assert actual["VLOGIC_3V3"]=={("U1","3"),("J2","1"),("J3","1"),("C2","1"),("C3","1"),("R1","1")}
    assert actual["LOGIC_RETURN_STAR"]=={("U1","2"),("J1","2"),("J2","2"),("J3","2"),
                                        ("C1","2"),("C2","2"),("C3","2"),("R1","2")}
    # The fuse is between J1 PACK input and regulator VIN; both nets MUST differ.
    assert "PACK_SWITCHED_3S_TBD" != "PACK_SWITCHED_LOGIC_FUSED"
    print("R13 native KiCad XML PASS: 4 exact nets, correct module pins, separate input fuse")


def static_power_contract():
    s=json.loads(SPEC.read_text())["source_specs"]
    # Bounds are 3S voltage envelope examples, NOT verified measured transient limits.
    for vin in (9.0,10.8,11.1,12.6):
        assert s["input_min_v"] <= vin <= s["input_max_v"]
    assert 0.5 * 1000 == s["max_output_ma"]
    min_load_ma=s["max_output_ma"] * s["min_recommended_load_percent_for_spec_ripple"]/100
    resistor_load_ma = 1000*s["output_v"]/300
    assert min_load_ma==10
    assert resistor_load_ma > min_load_ma
    resistor_mw=s["output_v"]**2/300*1000
    assert resistor_mw < 125  # OPTIONAL resistor, still temperature-rating TBD
    proposed_capacitance_uf=10 + .1  # Output only, excluding future downstream bypass
    assert proposed_capacitance_uf < s["maximum_capacitive_load_uF"]
    print("R13 nominal 3S voltage & optional preload arithmetic PASS; current/ripple/transients still unmeasured")


def check_erc(path):
    report=path.read_text(encoding="utf-8")
    issues=re.findall(r"^\[([^]]+)\]:",report,flags=re.MULTILINE)
    assert sorted(issues)==["power_pin_not_driven","power_pin_not_driven"],issues
    print("R13 KiCad ERC: EXACTLY TWO unresolved external battery input/return source errors; no waivers")


if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--xml",type=Path)
    parser.add_argument("--erc",type=Path)
    a=parser.parse_args()
    guard_source()
    static_power_contract()
    if a.xml:verify_xml(a.xml)
    if a.erc:check_erc(a.erc)
