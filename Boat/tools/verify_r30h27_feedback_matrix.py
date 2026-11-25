#!/usr/bin/env python3
"""R30H-27 non-actuating provenance matrix + source interlock audit.

A source ID, output-register readback or radio STOP=1 is NOT proof of load
current zero. Hardware reviewer signatures and measurements are not fabricated.
"""
from __future__ import annotations
import csv
import io
import sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
MATRIX="Boat/docs/BOAT_R30H_27_FEEDBACK_PROVENANCE_MATRIX.csv"
EXPECTED_IDS={f"H27-{i:02d}" for i in range(1,19)}
FIELDS={
 "Evidence_ID","Software_Field","Required_Function","Candidate_Source_Class",
 "Existing_Code_Origin","Commissioned_Source","Physical_Test_Point",
 "Active_Polarity","Max_Age_ms","Physical_Test_ID","Independent_Proof",
 "Physical_Result","Raw_Capture_SHA256","Reviewer","Review_Date","Failure_Action"
}
def audit(overrides:dict[str,str]|None=None)->list[str]:
 overrides=overrides or {}
 def rd(path:str)->str:
  return overrides[path] if path in overrides else (ROOT/path).read_text(encoding="utf-8")
 errors=[]
 try:
  f=csv.DictReader(io.StringIO(rd(MATRIX)))
  rows=list(f)
  if set(f.fieldnames or [])!=FIELDS:
   errors.append("provenance matrix schema changed")
  ids={r.get("Evidence_ID","") for r in rows}
  if len(rows)!=18 or ids!=EXPECTED_IDS:
   errors.append("R30H-27 requires 18 individually tracked evidence rows")
  for r in rows:
   k=r.get("Evidence_ID","?")
   if r.get("Physical_Result")!="NOT_RUN" or r.get("Raw_Capture_SHA256") or r.get("Reviewer") or r.get("Review_Date"):
    errors.append(f"{k}: inventing measured or signed acceptance")
   if r.get("Commissioned_Source") not in {"UNSELECTED","PROHIBITED","UNVERIFIED"}:
    errors.append(f"{k}: no physical source has been commissioned")
   if r.get("Independent_Proof") not in {"UNVERIFIED","NOT_INDEPENDENT"}:
    errors.append(f"{k}: cannot assert independent proof")
   if r.get("Active_Polarity") not in {"UNVERIFIED","N/A"}:
    errors.append(f"{k}: GPIO polarity is not physically established")
   if not r.get("Physical_Test_ID") or not r.get("Failure_Action"):
    errors.append(f"{k}: missing test reference or fail-closed action")
  by_id={r.get("Evidence_ID"):r for r in rows}
  for k in ("H27-05","H27-06","H27-08","H27-09","H27-16"):
   if "CURRENT" not in by_id.get(k,{}).get("Candidate_Source_Class",""):
    errors.append(f"{k}: independent current measurement required")
  for k in ("H27-17","H27-18"):
   if by_id.get(k,{}).get("Independent_Proof")!="NOT_INDEPENDENT":
    errors.append(f"{k}: software/GPIO echo must never certify STOP")
  if "INA231_BUS_ONLY" not in by_id.get("H27-07",{}).get("Candidate_Source_Class",""):
   errors.append("INA231 is selected for BUS voltage ONLY; not current-zero")
  if any("SEPARATE_MOTOR" in r.get("Required_Function","") for r in rows):
   errors.append("shared 3S bus must not invent separate motor power")
  hdr=rd("Boat/source/boat_feedback_provenance_audit.h")
  for req in (
     "PHYSICAL_FEEDBACK_SIGNOFF_COMPLETE = false",
     "Origin::SOFTWARE_COMMAND_ECHO",
     "Origin::MCP_OUTPUT_LATCH_READBACK",
     "Origin::EXTERNAL_CURRENT_MEASUREMENT",
     "Verdict::PHYSICAL_EVIDENCE_NOT_SIGNED",
     "return PHYSICAL_FEEDBACK_SIGNOFF_COMPLETE &&",
  ):
   if req not in hdr:errors.append(f"provenance code missing {req}")
  r30d=rd("Boat/source/boat_common_bus_readonly_port.h")
  r30c=rd("Boat/source/boat_common_bus_feedback.h")
  power=rd("Boat/source/boat_shared_bus_contract.h")
  main=rd("Boat/source/firmware/Flytec_Boat.ino")
  if "ACTUAL_READER_COMMISSIONED = false" not in r30d:errors.append("read-only physical reader lock broken")
  if "COMMON_BUS_HARDWARE_COMMISSIONED = false" not in r30c:errors.append("shared-bus commissioning lock broken")
  if "SHARED_BUS_PHYSICALLY_ACCEPTED = false" not in power:errors.append("shared 3S bus lock broken")
  if "SEPARATE_MOTOR_SOLENOID_POWER = false" not in power:errors.append("single 3S bus topology changed")
  if "application.attach(" in main:errors.append("uncommissioned physical driver attached")
  if "application.tick();" not in main:errors.append("diagnostic application missing")
  if "bool outputFeedbackValid,leftMotorActive,rightMotorActive;" not in rd("Boat/source/boat_hardware_supervisor.h"):
   errors.append("BoatHardwareSample source contract drift")
  if "if(stopped||!s.physicalStopReleased)" not in rd("Boat/source/boat_hardware_status.h"):
   errors.append("STOP wire inference requires separate provenance warning/review")
  gates=list(csv.DictReader(io.StringIO(rd("Boat/docs/BOAT_R30H_10_PHYSICAL_GATES.csv"))))
  if (len(gates)!=18 or sum(g.get("Status")=="NOT_RUN" for g in gates)!=12 or
      sum(g.get("Status")=="BLOCKED" for g in gates)!=6):
   errors.append("physical R30H-10 gates silently signed or changed")
  readiness=list(csv.DictReader(io.StringIO(rd("Boat/docs/BOAT_R30A_READINESS_GATES.csv"))))
  if len(readiness)!=24 or any(g.get("State")!="OPEN" for g in readiness):
   errors.append("R30A physical readiness changed without signed evidence")
 except (OSError,ValueError,csv.Error,KeyError) as e:
  errors.append(f"provenance matrix audit failed: {e}")
 return errors

if __name__=="__main__":
 errors=audit()
 for problem in errors:print("FAIL:",problem)
 if errors:sys.exit(2)
 print("R30H-27 18 physical provenance items UNSIGNED; 24 R30A and 18 R30H-10 gates locked; no output authority PASS")
