#!/usr/bin/env python3
"""No-net-fabrication checks for the preliminary KiCad R7 sheet.
This is NOT a KiCad ERC pass. Actual kicad-cli import/ERC is separate."""
import pathlib,re,unittest
f=pathlib.Path(__file__).resolve().parents[1]/"hardware"/"Direct_MDD20A_R7"/"Boat_MDD20A_Control_R7.kicad_sch"
s=f.read_text(encoding="utf8")
assert s.count("(kicad_sch ")==1
assert "(lib_symbols" in s and "(sheet_instances" in s
assert "(symbol \"Flytec:TPS3431\"" in s
for name in ("PWM_L","DIR_L","PWM_R","DIR_R","WDI","ACT_PERMIT","GATE_FEEDBACK","STOP_FEEDBACK","COIL_LH_IDLE","COIL_RK_IDLE"):
    assert s.count('(label "'+name+'"')>0,name
assert 'RP2040' not in s and 'SONAR' not in s
assert all(ref in s for ref in ['"J1"','"J2"','"J3"','"J4"','"J5"','"U1"'])
def balanced(text):
    depth=0;quote=False;escaped=False
    for ch in text:
        if escaped:escaped=False;continue
        if ch=="\\" and quote:escaped=True;continue
        if ch=='"':quote=not quote;continue
        if not quote:
            if ch=='(':depth+=1
            if ch==')':depth-=1
            if depth<0:return False
    return depth==0 and not quote
assert balanced(s)
print("Preliminary KiCad R7 source structure and signals OK (not a tool ERC)")
