#!/usr/bin/env python3
"""R30C: static fail-closed proof for selected one-common-3S bus safety model.

This tests source/wiring/doc consistency only, NOT a real safety controller.
"""
import csv
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
read=lambda p:(ROOT/p).read_text(encoding="utf-8")
code=read("Boat/source/boat_common_bus_feedback_r30c.h")
for token in (
    "COMMON_BUS_HARDWARE_COMMISSIONED = false",
    "commonDisconnectClosedFeedback",
    "commonDisconnectDcRatingSigned",
    "commonDisconnectPowerOffTestSigned",
    "independentlySupervisedWatchdogHealthy",
    "coilOneHotPulseLimiterCommissioned",
    "independentlyObservedCoilCurrentZero",
    "independentlyObservedMotorCurrentZero",
    "localBatteryHealthy",
    "MAX_FEEDBACK_AGE_MS = 50U",
    "MAX_COMMAND_LEASE_MS = 100U",
    "MAX_COIL_PULSE_MS = 500U",
    "return COMMON_BUS_HARDWARE_COMMISSIONED &&",
    "boat_r29::exclusiveActuatorRequest(r)",
    "Review::DESIGN_EVIDENCE_CONSISTENT",
):
    assert token in code, token
legacy=read("Boat/source/physical_driver_commissioning_gate.h")
assert "Do NOT short, duplicate, alias or fake" in legacy
adapter=read("Boat/source/boat_physical_driver_adapter.h")
assert "!independentHardwareCommissioned" in adapter
assert "motorContactorFeedbackClosed" in adapter and "coilContactorFeedbackClosed" in adapter
firmware=read("Boat/source/firmware/Flytec_Boat.ino")
assert "application.attach(" not in firmware and "application.tick();" in firmware
r29=read("Boat/source/boat_r29_shared_bus_contract.h")
assert "SHARED_3S_PACK = true" in r29
assert "SEPARATE_MOTOR_SOLENOID_POWER = false" in r29
assert "SHARED_BUS_PHYSICALLY_ACCEPTED = false" in r29
with (ROOT/"Boat/docs/BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv").open(newline="",encoding="utf-8") as f:
    wires={x["Net"]:x for x in csv.DictReader(f)}
assert wires["MDD_MOTOR_VM"]["Source"] == wires["EKM_FET_VH"]["Source"] == "COMMON_3S_DISTRIBUTION"
assert wires["MDD_MOTOR_GND"]["Source"] == wires["EKM_FET_GND"]["Source"] == "COMMON_3S_DISTRIBUTION"
assert wires["MDD_MOTOR_VM"]["Install_Status"] == wires["EKM_FET_VH"]["Install_Status"] == "UNWIRED_PENDING"
with (ROOT/"Boat/docs/BOAT_R30A_READINESS_GATES.csv").open(newline="",encoding="utf-8") as f:
    gates=list(csv.DictReader(f))
assert len(gates)==24 and all(g["State"]=="OPEN" for g in gates)
assert {"G11","G13","G14","G15","G17"}.issubset({g["Gate_ID"] for g in gates})
assert "INA231_PHYSICAL_COMMISSIONED=false" in read("Boat/source/ina231_voltage_contract_r27.h")
doc=read("Boat/docs/BOAT_R30C_ONE_BUS_SAFETY_FEEDBACK_CONTRACT.md")
assert "NOT RUN" in doc and "Do NOT refactor the old dual-contactor" in doc
print("R30C one-shared-bus STOP/watchdog feedback design PASS (host audit; physical actuation LOCKED)")
