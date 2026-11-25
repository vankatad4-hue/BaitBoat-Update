#!/usr/bin/env python3
"""R30B software guard regression: fail closed without authorizing hardware."""
import csv
from pathlib import Path
root=Path(__file__).resolve().parents[2]
read=lambda p:(root/p).read_text(encoding="utf-8")
guard=read("Boat/source/boat_shared_supply_fault_guard.h")
app=read("Boat/source/boat_application_port.cpp")
faults=read("Boat/tests/boat_r30b_shared_supply_fault_matrix_test.cpp")
assert "STOP_INVALID_OR_CRITICAL" in guard
assert "snapshot.valid" in guard
assert "snapshot.critical" in guard
assert "sensorVoltageValid" in guard
assert "measuredAtMs" in guard
assert "BoatBatteryMonitor::MAX_SAMPLE_AGE_MS" in guard
assert "canRequestConditionalHome" in guard
assert '#include "boat_shared_supply_fault_guard.h"' in app
assert "boat_r30b::inspectBoatBattery(" in app
assert "sample_.batteryHealthy=energy!=boat_r30b::EnergyEvidence::STOP_INVALID_OR_CRITICAL" in app
assert "fault();radio_.update();" in app
assert "boatRadioLossRthDue" in faults
assert "BoatPhysicalDriverAdapter real;" in faults
assert "!real.begin(uncommissioned,pulse,false)" in faults
assert "INA231_PHYSICAL_COMMISSIONED=false" in read("Boat/source/ina231_voltage_contract.h")
assert "SHARED_BUS_PHYSICALLY_ACCEPTED = false" in read("Boat/source/boat_shared_bus_contract.h")
assert "SEPARATE_MOTOR_SOLENOID_POWER = false" in read("Boat/source/boat_shared_bus_contract.h")
assert "!independentHardwareCommissioned" in read("Boat/source/boat_physical_driver_adapter.h")
with (root/"Boat/docs/BOAT_R30A_READINESS_GATES.csv").open(newline="",encoding="utf-8") as f:
    gates=list(csv.DictReader(f))
assert len(gates)==24 and all(g["State"]=="OPEN" for g in gates)
print("R30B runtime energy guard and unsigned hardware interlocks PASS (source audit only)")
