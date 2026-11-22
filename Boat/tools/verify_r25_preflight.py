#!/usr/bin/env python3
"""R25 fail-closed review of physical evidence. Does not access GPIO or loads."""
import argparse
import csv
import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SHEET = ROOT / "Boat/docs/BOAT_R25_POWER_LEVEL_EVIDENCE.csv"
COLUMNS = ("Check_ID","Status","Measured_V","Board_Revision","Operator","Date","Evidence_Path","Notes")
# Conservative screening bounds, not substitutes for manufacturer operating limits.
LIMITS = {
 "MAIN_5V_LOAD": (4.75, 5.25),
 "EKM_LOGIC_5V_LOAD": (4.75, 5.25),
 "NRF_3V3_LOAD": (3.135, 3.465),
 "SENSOR_3V3_LOAD": (3.135, 3.465),
 "M100_TX_HIGH_ISOLATED": (2.5, 3.3),
 "M100_SDA_HIGH_ISOLATED": (2.5, 3.3),
 "M100_SCL_HIGH_ISOLATED": (2.5, 3.3),
 "E22_TXD_HIGH_ISOLATED": (2.5, 3.3),
 "E22_AUX_HIGH_ISOLATED": (2.5, 3.3),
 "I2C_PULLUP_HIGH": (3.0, 3.3),
 "ADC_POWERUP_PEAK_AIN_MINUS_VDD": (-0.25, 0.25),
 "ADC_POWERDOWN_PEAK_AIN_MINUS_VDD": (-0.25, 0.25),
 "PWM39_BOOT_MAX_V": (-0.05, 0.25),
 "PWM40_BOOT_MAX_V": (-0.05, 0.25),
 "EKM_IN1_BOOT_MAX_V": (-0.05, 0.25),
 "EKM_IN2_BOOT_MAX_V": (-0.05, 0.25),
 "EKM_IN3_BOOT_MAX_V": (-0.05, 0.25),
 "EKM_IN4_BOOT_MAX_V": (-0.05, 0.25),
}
MANUAL = (
 "DEVKITC_REV_AND_WROOM2_ID",
 "M100_SH1_CAVITY_AUDIT",
 "E22_26PAD_CARRIER_AUDIT",
 "NRF_HEADER_POLARITY_AUDIT",
 "MCP_ADDRESS_RESET_AUDIT",
 "HIGH_CURRENT_PATHS_DISCONNECTED",
 "INDEPENDENT_STOP_CUTOFF_ACCEPTED",
 "INDEPENDENT_WATCHDOG_ACCEPTED",
 "INDEPENDENT_COIL_CURRENT_ZERO_ACCEPTED",
 "INDEPENDENT_COIL_PULSE_LIMIT_ACCEPTED",
)
IDS = tuple(LIMITS) + MANUAL

def validate(rows, root=ROOT, require_blank=False):
    """Return blocking problems; an empty error list is NOT a safety certificate."""
    problems=[]
    actual=[row.get("Check_ID","") for row in rows]
    if len(actual)!=len(IDS) or set(actual)!=set(IDS) or len(actual)!=len(set(actual)):
        problems.append("R25 IDs missing, extra or duplicated")
    for row in rows:
        name=row.get("Check_ID","")
        if name not in IDS: continue
        status=row.get("Status","")
        if status not in ("NOT_RUN","PASS","FAIL"):
            problems.append(name+": invalid status")
            continue
        if require_blank and status!="NOT_RUN":
            problems.append(name+": cannot sign physical commissioning in CI baseline")
        if status=="NOT_RUN":
            if any(row.get(k,"").strip() for k in COLUMNS[2:7]):
                problems.append(name+": unperformed check has measurement/signature")
            continue
        if not all(row.get(k,"").strip() for k in ("Board_Revision","Operator","Date","Evidence_Path")):
            problems.append(name+": missing revision/operator/date/evidence")
            continue
        try: datetime.date.fromisoformat(row["Date"])
        except ValueError: problems.append(name+": date is not YYYY-MM-DD")
        evidence=row["Evidence_Path"]
        if not evidence.startswith("Boat/evidence/r25/") or ".." in Path(evidence).parts:
            problems.append(name+": invalid evidence path")
        elif not (root/evidence).is_file():
            problems.append(name+": evidence file missing")
        if name in LIMITS:
            try:
                v=float(row.get("Measured_V",""))
                lo,hi=LIMITS[name]
                if not lo<=v<=hi: problems.append(name+": measured voltage outside screening interval")
            except (TypeError,ValueError):
                problems.append(name+": numeric voltage missing")
        elif row.get("Measured_V","").strip():
            problems.append(name+": manual check must not invent a voltage")
        if status=="FAIL":
            problems.append(name+": reported FAIL")
    return problems

def load(path):
    with path.open(newline="",encoding="utf-8") as f:
        reader=csv.DictReader(f)
        if tuple(reader.fieldnames or ())!=COLUMNS:
            raise ValueError("R25 CSV columns changed")
        return list(reader)

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--baseline",action="store_true",help="CI requires every physical check NOT_RUN")
    ap.add_argument("--evaluate",action="store_true",help="require all physical check rows PASS")
    ap.add_argument("--file",type=Path,default=SHEET)
    args=ap.parse_args()
    try:
        rows=load(args.file)
        problems=validate(rows,root=ROOT,require_blank=args.baseline)
        if args.evaluate:
            unrun=[r for r in rows if r.get("Status")!="PASS"]
            if unrun: problems.append("NOT ACCEPTED: "+str(len(unrun))+" rows not PASS")
        for problem in problems: print("BLOCKED:",problem)
        if problems: return 1
        if args.baseline: print("R25 CI baseline valid: all physical checks NOT_RUN; commissioning BLOCKED")
        elif args.evaluate: print("R25 evidence structurally valid; independent human safety review STILL REQUIRED")
        else: print("R25 structure valid; this DOES NOT certify physical hardware")
        return 0
    except (ValueError,OSError) as error:
        print("BLOCKED:",error)
        return 1

if __name__=="__main__": raise SystemExit(main())
