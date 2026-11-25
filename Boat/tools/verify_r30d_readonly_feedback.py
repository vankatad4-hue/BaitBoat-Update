#!/usr/bin/env python3
"""R30D read-only common-bus observation contract and uncommissioned gates."""
import csv
from pathlib import Path
root=Path(__file__).resolve().parents[2]
rd=lambda path:(root/path).read_text(encoding="utf-8")
port=rd("Boat/source/boat_common_bus_readonly_port.h")
check=rd("Boat/source/boat_common_bus_feedback.h")
firmware=rd("Boat/source/firmware/Flytec_Boat.ino")
test=rd("Boat/tests/boat_r30d_common_bus_readonly_faults_test.cpp")
assert "ACTUAL_READER_COMMISSIONED = false" in port
assert "COMMON_BUS_HARDWARE_COMMISSIONED = false" in check
assert "return COMMON_BUS_HARDWARE_COMMISSIONED &&" in check
assert "INA231_PHYSICAL_COMMISSIONED=false" in rd("Boat/source/ina231_voltage_contract.h")
assert "SHARED_BUS_PHYSICALLY_ACCEPTED = false" in rd("Boat/source/boat_shared_bus_contract.h")
assert "SEPARATE_MOTOR_SOLENOID_POWER = false" in rd("Boat/source/boat_shared_bus_contract.h")
assert 'boat_common_bus_readonly_port.h' in firmware
assert "static_assert(!boat_r30d::ACTUAL_READER_COMMISSIONED" in firmware
assert "application.attach(" not in firmware
assert "application.tick();" in firmware
assert "virtual bool read(RawFrame &out) = 0" in port
assert "RawFrame raw{}" in port
assert "canAuthorizeActuators = false" in port
assert "f.commonDisconnectDcRatingSigned=false" in port
assert "f.commonDisconnectPowerOffTestSigned=false" in port
assert "f.coilOneHotPulseLimiterCommissioned=false" in port
assert "f.independentlyObservedCoilCurrentZero=false" in port
assert "f.independentlyObservedMotorCurrentZero=false" in port
assert "f.localBatteryHealthy=boat_r27::INA231_PHYSICAL_COMMISSIONED" in port
assert "Result::DUPLICATE_SOURCE" in port
assert "Result::SOURCE_UNASSIGNED" in port
assert "Result::STALE_SAFETY" in port
assert "Result::STALE_BATTERY" in port
assert "Result::INVALID_BATTERY" in port
assert "static_assert(!ACTUAL_READER_COMMISSIONED" in test
assert "assert(!physical.begin(nonexistent,pulses,false))" in test
# Adapter has no production write/enable/actuation method; only read().
src=port[port.find("class ReadOnlySource"):port.find("enum class Result")]
# Ignore explanatory "//" comments; check only actual API declarations.
api_only="\n".join(line.split("//",1)[0] for line in src.splitlines())
for bad in ("write(", "motor(", "coil(", "enable(", "clearStop(", "permit(", "cutOutputs("):
    assert bad not in api_only, bad
assert "!independentHardwareCommissioned" in rd("Boat/source/boat_physical_driver_adapter.h")
with (root/"Boat/docs/BOAT_R30A_READINESS_GATES.csv").open(newline="",encoding="utf-8") as f:
    gates=list(csv.DictReader(f))
assert len(gates)==24 and all(g["State"]=="OPEN" for g in gates)
doc=rd("Boat/docs/BOAT_R30D_READONLY_FEEDBACK_SIMULATION.md")
assert "NOT COMMISSIONED" in doc and "one common 3S" in doc
print("R30D read-only transport contract and 24 physical lockouts PASS (source audit; no actuators)")
