#!/usr/bin/env python3
"""R30H-9 hardware-independent, FAIL-CLOSED no-GPIO marker preflight.

This is NOT a hardware authorization, RF timing test, or actuator STOP proof.
The source-owned worksheets must remain unsigned; no code can select a
diagnostic GPIO until separately reviewed physical evidence is available.
"""
import csv
import io
import pathlib
import re

ROOT=pathlib.Path(__file__).resolve().parents[2]
PATHS=(
 "Boat/source/diagnostic_marker_contract.h",
 "Remote/source/diagnostic_marker_contract.h",
 "Boat/source/boat_diagnostic_marker_config.h",
 "Remote/source/remote_diagnostic_marker_config.h",
 "Boat/source/boat_dual_radio_port.h",
 "Boat/source/boat_local_stop_trace.h",
 "Remote/source/boat_radio.cpp",
 "Remote/source/remote_stop_trace.h",
 "Boat/docs/BOAT_R30H_9_PROBE_CROSSWALK.csv",
 "Boat/docs/BOAT_R30H_9_PHYSICAL_SIGNOFF.csv",
)
STOP_OBSERVABLES=("STOP_AUTH_RECEIVED","STOP_LOGICAL_INTENT","RTH_LOCAL_ENTRY")
BOAT_LOCKS=("#define BOAT_R30H9_MARKER_GPIO (-1)",
            "#define BOAT_R30H9_GPIO_OUTPUT_SIGNED 0")
REMOTE_LOCKS=("#define REMOTE_R30H9_MARKER_GPIO (-1)",
              "#define REMOTE_R30H9_GPIO_OUTPUT_SIGNED 0")
DRIVE_TOKENS=("pinMode(","digitalWrite(","gpio_set_level(","gpio_config(","ledcAttach(","gpio_reset_pin(")

def snapshot(root=ROOT):
 return {p:(root/p).read_text(encoding="utf-8") for p in PATHS}

def verify(data):
 errors=[]
 for p in PATHS:
  if p not in data:
   errors.append(f"R30H-9 required path missing {p}")
 if errors:return errors
 def require(path,*words):
  for word in words:
   if word not in data[path]:errors.append(f"{path}: missing expected {word}")
 boat="Boat/source/boat_diagnostic_marker_config.h"
 remote="Remote/source/remote_diagnostic_marker_config.h"
 for p,locks in ((boat,BOAT_LOCKS),(remote,REMOTE_LOCKS)):
  require(p,*locks,"#error")
  if any(t in data[p] for t in DRIVE_TOKENS):
   errors.append(f"{p}: GPIO driver code prohibited in unsigned marker configuration")
 contract="Boat/source/diagnostic_marker_contract.h"
 mirrored="Remote/source/diagnostic_marker_contract.h"
 if data[contract]!=data[mirrored]:
  errors.append("R30H-9 Boat and Remote marker contract headers must match byte-for-byte")
 require(contract,"inline bool currentlyApprovedOutput(Endpoint, int) {return false;}",
         "isBoatReservedForSafety","isRemoteKnownOccupiedOrRestricted",
         "SOFTWARE_LOCAL_UNCERTIFIED")
 for p in (contract,mirrored):
  if any(t in data[p] for t in DRIVE_TOKENS):
   errors.append(f"{p}: no physical IO, timers or GPIO allowed in recorder")
 require("Boat/source/boat_dual_radio_port.h",
         '#include "boat_diagnostic_marker_config.h"',
         '#include "diagnostic_marker_contract.h"',
         "BOAT_R30H9_MARKER_GPIO == -1",
         "BOAT_R30H9_GPIO_OUTPUT_SIGNED == 0")
 require("Remote/source/boat_radio.cpp",
         '#include "remote_diagnostic_marker_config.h"',
         '#include "diagnostic_marker_contract.h"',
         "REMOTE_R30H9_MARKER_GPIO == -1",
         "REMOTE_R30H9_GPIO_OUTPUT_SIGNED == 0")
 require("Boat/source/boat_local_stop_trace.h",*STOP_OBSERVABLES)
 require("Remote/source/remote_stop_trace.h",
         "STOP_BUTTON_REQUEST","NRF_BUFFER_ADMITTED",
         "E22_UART_BUFFER_ADMITTED")
 probes=list(csv.DictReader(io.StringIO(data["Boat/docs/BOAT_R30H_9_PROBE_CROSSWALK.csv"])))
 if (len(probes)!=10 or
     {r.get("Probe_ID") for r in probes}!={f"H9-{i:02d}" for i in range(1,11)}):
  errors.append("R30H-9 probe worksheet must have ten exact probe IDs")
 expected_passive={f"H9-{i:02d}" for i in range(1,6)}
 for row in probes:
  gid=row.get("Probe_ID","?")
  if row.get("State")!="NOT_RUN" or any(row.get(n) for n in
      ("Measured_Voltage_V","Actual_Header_Contact","Capture_Evidence","Engineer_Signoff")):
   errors.append(f"{gid}: physical probe measurement or signature forged")
  if gid in expected_passive:
   if not row.get("Probe_Class","").startswith("PASSIVE_"):
    errors.append(f"{gid}: existing signal cannot become an unreviewed MCU output")
  else:
   if (row.get("Pin_Proposal")!="UNASSIGNED" or
       row.get("Probe_Class")!="ACTIVE_DIAGNOSTIC_OUTPUT_NOT_IMPLEMENTED"):
    errors.append(f"{gid}: dynamic GPIO output cannot be commissioned by preflight")
  if "GPIO1/2/41/42" in row.get("Pin_Proposal",""):
   errors.append(f"{gid}: safety-reserved Boat pins cannot be stolen")
 physical=list(csv.DictReader(io.StringIO(data["Boat/docs/BOAT_R30H_9_PHYSICAL_SIGNOFF.csv"])))
 if len(physical)!=12 or {r.get("Gate_ID") for r in physical}!={f"H9-P{i:02d}" for i in range(1,13)}:
  errors.append("R30H-9 needs twelve unsigned hardware gates")
 for r in physical:
  if r.get("Status")!="NOT_RUN" or any(r.get(n) for n in
      ("Observation","Evidence","Reviewer")):
   errors.append(f"{r.get('Gate_ID')}: real commissioning cannot be claimed in source CI")
 return errors

def main():
 try:
  errors=verify(snapshot())
 except (OSError, UnicodeError) as exc:
  print("R30H-9 source read failure:",exc)
  return 2
 if errors:
  for e in errors:print("R30H-9 FAIL:",e)
  return 1
 print("R30H-9 PASS SOURCE ONLY: GPIO=-1, output signoff=0; 10 probes + 12 hardware gates NOT_RUN")
 print("NO RF measurements, no PPS, no electrical marker, no current-zero, no STOP approval")
 return 0

if __name__=="__main__":
 raise SystemExit(main())
