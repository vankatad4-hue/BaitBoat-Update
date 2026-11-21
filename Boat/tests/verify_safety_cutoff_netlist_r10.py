#!/usr/bin/env python3
"""R10 KiCad native netlist topology only. Not electrical safety qualification."""
from pathlib import Path
from collections import defaultdict
import argparse
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
SCH = ROOT / "hardware/Safety_Cutoff_R10/Boat_Safety_Cutoff_R10.kicad_sch"
EXPECTED_REFS = {"J1","J2","J3","J4","J5","J6","U1","U2","U3","U4",
                 "Q1","Q2","R1","R2","R3","R4","F1","F2","F3","S1"}
SAFE_IN = ("STOP_OK_NC_SECOND_POLE","WDO_N","UVLO_OK","CURRENT_OK",
           "TEMP_OK","RAIL_OK","MOTOR_IDLE","COIL_IDLE","OUTPUT_FB_OK","POR_OK")
PIN_AND = ((1,2,3),(4,5,6),(9,10,8),(12,13,11))
GATES = [
  (SAFE_IN[0],SAFE_IN[1],"SAFE_1"),
  ("SAFE_1",SAFE_IN[2],"SAFE_2"),
  ("SAFE_2",SAFE_IN[3],"SAFE_3"),
  ("SAFE_3",SAFE_IN[4],"SAFE_4"),
  ("SAFE_4",SAFE_IN[5],"SAFE_5"),
  ("SAFE_5",SAFE_IN[6],"SAFE_6"),
  ("SAFE_6",SAFE_IN[7],"SAFE_7"),
  ("SAFE_7",SAFE_IN[8],"SAFE_8"),
  ("SAFE_8",SAFE_IN[9],"ALL_SAFE_CLEAR_N"),
  ("LATCH_Q","MCU_PERMIT","DRIVE_ALLOWED"),
  ("PACK_RETURN","PACK_RETURN",None),
  ("PACK_RETURN","PACK_RETURN",None),
]

def source_check():
    s = SCH.read_text()
    refs = re.findall(r'\(symbol \(lib_id "FlytecR10:[^"]+"\).*?\(property "Reference" "([^"]+)"', s)
    assert len(refs) == len(EXPECTED_REFS) == 20
    assert set(refs) == EXPECTED_REFS, set(refs)
    assert s.count('(property "Footprint" ""') == 20
    assert 'R10-NO-FABRICATION' in s
    assert 'ARM_RISING_EDGE_RELEASE_QUALIFIED_TBD' in s
    assert 'PMOS_HIGH_SIDE_SOA_TBD' in s
    assert 'RP2040' not in s and 'SONAR' not in s and 'E22' not in s
    print("R10 schematic source guard passed: 20 unqualified parts, no footprints")

def expected():
    n = defaultdict(set)
    def put(net,ref,pin): n[net].add((ref,str(pin)))
    # power/safety interfaces
    for ref,ps in {
      "J1":("PACK_SWITCHED","PACK_RETURN"),
      "J2":("VLOGIC_3V3","PACK_RETURN"),
      "J3":SAFE_IN,
      "J4":("MCU_PERMIT","ARM_RISING_EDGE_RELEASE_QUALIFIED_TBD","PACK_RETURN","VLOGIC_3V3"),
      "J5":("MOTOR_POS_FUSED","PACK_RETURN"),
      "J6":("ACT_SOL_POS_AFTER_LATCH_TBD","PACK_RETURN"),
      "U4":( # 1..14, NC 6/8/9
          "ALL_SAFE_CLEAR_N","VLOGIC_3V3","ARM_RISING_EDGE_RELEASE_QUALIFIED_TBD",
          "VLOGIC_3V3","LATCH_Q",None,"PACK_RETURN",None,None,
          "VLOGIC_3V3","PACK_RETURN","PACK_RETURN","VLOGIC_3V3","VLOGIC_3V3"
      ),
      "Q1":("HIGH_SIDE_GATE","PACK_FUSED","ACTUATOR_BUS"),
      "Q2":("DRIVE_ALLOWED","HIGH_SIDE_GATE","STOP_SOURCE_RETURN"),
      "S1":("STOP_SOURCE_RETURN","PACK_RETURN"),
      "F1":("PACK_SWITCHED","PACK_FUSED"),
      "F2":("ACTUATOR_BUS","MOTOR_POS_FUSED"),
      "F3":("ACTUATOR_BUS","ACT_SOL_POS_AFTER_LATCH_TBD"),
      "R1":("VLOGIC_3V3","WDO_N"),
      "R2":("PACK_FUSED","HIGH_SIDE_GATE"),
      "R3":("DRIVE_ALLOWED","PACK_RETURN"),
      "R4":("ACTUATOR_BUS","PACK_RETURN"),
    }.items():
      for idx,net in enumerate(ps,1):
        if net:put(net,ref,idx)
    for i,(a,b,c) in enumerate(GATES):
      ref = "U" + str(i//4+1)
      pa,pb,pc = PIN_AND[i%4]
      put(a,ref,pa);put(b,ref,pb)
      if c:put(c,ref,pc)
    for u in ("U1","U2","U3"):
      put("PACK_RETURN",u,7);put("VLOGIC_3V3",u,14)
    return dict(n)

def check_xml(path):
    doc=ET.parse(path)
    actual={}
    for el in doc.findall("./nets/net"):
      name=el.get("name","").removeprefix("/")
      assert name not in actual, name
      actual[name] = {(x.get("ref"),x.get("pin")) for x in el.findall("node")}
    want=expected()
    # KiCad emits synthetic nets for explicitly no-connected unused outputs.
    # Audit precisely those pins, not arbitrary floating/unconnected inputs.
    allowed_nc = {("U3","8"),("U3","11"),("U4","6"),("U4","8"),("U4","9")}
    nc_nets = {name: nodes for name,nodes in actual.items()
               if name.startswith("unconnected-")}
    assert len(nc_nets)==len(allowed_nc)
    seen=set().union(*(nodes for nodes in nc_nets.values()))
    assert seen==allowed_nc, (seen,allowed_nc)
    assert all(len(nodes)==1 for nodes in nc_nets.values())
    actual={name:nodes for name,nodes in actual.items() if name not in nc_nets}
    assert set(actual)==set(want), "Unexpected/missing nets: "+str(sorted(set(actual)^set(want)))
    for name in sorted(want):
      assert actual[name]==want[name], f"{name}: got {sorted(actual[name])}; expected {sorted(want[name])}"
    assert actual["STOP_SOURCE_RETURN"]=={("Q2","3"),("S1","1")}
    assert actual["HIGH_SIDE_GATE"]=={("Q1","1"),("Q2","2"),("R2","2")}
    assert actual["ALL_SAFE_CLEAR_N"]=={("U3","3"),("U4","1")}
    assert actual["WDO_N"]=={("J3","2"),("R1","2"),("U1","2")}
    assert actual["ACTUATOR_BUS"]=={("Q1","3"),("F2","1"),("F3","1"),("R4","1")}
    print(f"Native KiCad R10 XML exact pin topology passed: {len(actual)} nets")
    print("HARDWARE BLOCKED: conditioned STOP, comparators, ARM filter, FET ratings, coils, R7/ERC, physical acceptance")

if __name__=="__main__":
    ap=argparse.ArgumentParser()
    ap.add_argument("--xml",type=Path)
    args=ap.parse_args()
    source_check()
    if args.xml:check_xml(args.xml)
