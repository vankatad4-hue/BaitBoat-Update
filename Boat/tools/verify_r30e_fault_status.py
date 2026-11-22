#!/usr/bin/env python3
"""R30E diagnostic-only status transport. Never relabel as RF safety permission."""
from pathlib import Path
import csv
root=Path(__file__).resolve().parents[2]
get=lambda p:(root/p).read_text(encoding="utf-8")
status=get("Boat/source/boat_r30e_fault_status.h")
app=get("Boat/source/firmware/Flytec_Boat.ino")
test=get("Boat/tests/boat_r30e_fault_status_test.cpp")
for token in ("NO_READER","SOURCE_DUPLICATED","BATTERY_EXPIRED",
              "CONTACT_EXPIRED","STOP_OPEN","WATCHDOG_LOST",
              "DISCONNECT_OPEN","LIMITER_NOT_READY","COIL_CURRENT_NONZERO",
              "MOTOR_CURRENT_NONZERO","UNCOMMISSIONED",
              "safeToActuate=false","0xD5","encodeLocal",
              "UNVERIFIED|COMMISSIONING_LOCK"):
    assert token in status,token
assert 'boat_r30e_fault_status.h' in app
assert "safetyObservations.snapshot(" in app
assert "boat_r30e::makeStatus(" in app
assert "boat_r30e::encodeLocal(" in app
assert "Serial.printf(\"R30E,LOCAL_ONLY" in app
assert "outputs=LOCKED" in app
assert "application.attach(" not in app
assert "application.tick();" in app
assert "radio.sendHardwareTelemetry(status)" not in app
# Radio wire enums remain untouched; no private local diagnostic subtype
# may be silently transmitted as a STATUS or other authenticated payload.
wire=get("Boat/source/radio_payload_codec.h")
assert "RADIO_PAYLOAD_R30E" not in wire
assert "RADIO_PAYLOAD_DIAGNOSTIC" not in wire
port=get("Boat/source/boat_common_bus_readonly_port_r30d.h")
assert "ACTUAL_READER_COMMISSIONED = false" in port
assert "COMMON_BUS_HARDWARE_COMMISSIONED = false" in get("Boat/source/boat_common_bus_feedback_r30c.h")
assert "SHARED_BUS_PHYSICALLY_ACCEPTED = false" in get("Boat/source/boat_r29_shared_bus_contract.h")
assert "SEPARATE_MOTOR_SOLENOID_POWER = false" in get("Boat/source/boat_r29_shared_bus_contract.h")
assert "INA231_PHYSICAL_COMMISSIONED=false" in get("Boat/source/ina231_voltage_contract_r27.h")
assert "assert(!physical.begin(unavailable,pulses,false))" in test
with (root/"Boat/docs/BOAT_R30A_READINESS_GATES.csv").open(newline="",encoding="utf-8") as f:
    gates=list(csv.DictReader(f))
assert len(gates)==24 and all(row["State"]=="OPEN" for row in gates)
doc=get("Boat/docs/BOAT_R30E_LOCAL_FAULT_TELEMETRY.md")
assert "NOT A RADIO PAYLOAD" in doc
assert "NO_READER" in doc
print("R30E source audit PASS: local-only fault telemetry; no RF safety status/actuator unlock")
