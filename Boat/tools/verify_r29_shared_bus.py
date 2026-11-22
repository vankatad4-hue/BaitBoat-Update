#!/usr/bin/env python3
"""R29 chosen single 3S battery bus: topology and output lockout oracle."""
from pathlib import Path
import csv
ROOT=Path(__file__).resolve().parents[2]
def get(p):return (ROOT/p).read_text(encoding="utf-8")
with (ROOT/"Boat/docs/BOAT_R29_SHARED_3S_BUS_CONNECTIONS.csv").open(newline="",encoding="utf-8") as f:
    rows=list(csv.DictReader(f))
assert len(rows)==8 and all(r["Circuit_ID"]=="COMMON_3S" for r in rows)
assert all(r["Connection_Status"]=="UNWIRED_PENDING" for r in rows)
nets={r["Net"]:r for r in rows}
assert len(nets)==8
assert nets["MDD_MOTOR_VM"]["Source"]=="COMMON_3S_DISTRIBUTION"
assert nets["EKM_FET_VH"]["Source"]=="COMMON_3S_DISTRIBUTION"
assert nets["MDD_MOTOR_VM"]["Source_Contact"]==nets["EKM_FET_VH"]["Source_Contact"]=="POS"
assert nets["MDD_MOTOR_GND"]["Source"]==nets["EKM_FET_GND"]["Source"]=="COMMON_3S_DISTRIBUTION"
assert nets["INA231_BUS_SENSE"]["Source_Contact"]=="PROTECTED_LOW_CURRENT_SENSE"
assert nets["EKM_LOGIC_VL"]["Voltage_Domain"]=="5V_LOGIC"
purchase=get("docs/PROJECT_PURCHASE_REGISTER_R20.csv")
assert "VH remains separate 3S pack" not in purchase
assert "VH and Cytron MDD20A VM share the same common 3S4P battery distribution bus" in purchase
code=get("Boat/source/boat_r29_shared_bus_contract.h")
assert "SHARED_3S_PACK = true" in code
assert "SEPARATE_MOTOR_SOLENOID_POWER = false" in code
assert "SHARED_BUS_PHYSICALLY_ACCEPTED = false" in code
assert "exclusiveActuatorRequest" in code
driver=get("Boat/source/boat_physical_driver_adapter.h")
assert "exclusiveActuatorRequest(request)" in driver
assert "!independentHardwareCommissioned" in driver
ina=get("Boat/source/ina231_voltage_contract_r27.h")
assert "INA231_PHYSICAL_COMMISSIONED=false" in ina
sensor=get("Boat/source/tracker_sensor_profile.example.h")
assert "c.ina231BoardAndWiringSigned=false" in sensor
assert "c.ina231DmmCalibrationSigned=false" in sensor
for fn,count in [("Boat/docs/BOAT_R23_GPIO_AND_LEVEL_SIGNOFF.csv",32),
                 ("Boat/docs/BOAT_R25_POWER_LEVEL_EVIDENCE.csv",28),
                 ("Boat/docs/BOAT_R27_INA231_ACCEPTANCE.csv",21)]:
    with (ROOT/fn).open(newline="",encoding="utf-8") as f:
        tests=list(csv.DictReader(f))
    assert len(tests)==count and all(t["Status"]=="NOT_RUN" for t in tests)
print("R29 shared 3S bus policy and 81 physical NOT_RUN safeguards verified; no motor/solenoid hardware commissioned")
