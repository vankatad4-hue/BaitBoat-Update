#!/usr/bin/env python3
"""R30F joint Boat/Remote diagnostic telemetry contract: source-only audit.

This is protocol static analysis and CI compilation, never physical RF
authentication or independent emergency power isolation signoff.
"""
from pathlib import Path
import csv
root=Path(__file__).resolve().parents[2]
def read(p): return (root/p).read_text(encoding="utf-8")
shared=read("Boat/source/radio_safety_diagnostic_v1.h")
assert shared==read("Remote/source/radio_safety_diagnostic_v1.h")
for token in ("SUBTYPE = 8", "VERSION = 1", "FLAGS_UNVERIFIED = 0x0002U",
              "FLAGS_COMMISSIONING_LOCK = 0x0800U", "FLAGS_DEFINED_MASK = 0x0FFFU",
              "MAX_REASON = 14", "millivolts != 0xFFFFU", "observed != (d.reason >= 8U)"):
    assert token in shared,token
for end in ("Boat", "Remote"):
    head=read(f"{end}/source/radio_payload_codec.h")
    decoder=read(f"{end}/source/radio_payload_codec.cpp")
    encoder=read(f"{end}/source/radio_payload_encoder.cpp")
    assert "RADIO_PAYLOAD_SAFETY_DIAGNOSTIC" in head
    assert "diagnosticReason" in head and "diagnosticFlags" in head
    assert "case 8:" in decoder and "radio_r30f::decode(p,diagnostic)" in decoder
    assert "case RADIO_PAYLOAD_SAFETY_DIAGNOSTIC:" in encoder
    assert "radio_r30f::encode(d,out)" in encoder
    assert (root/f"{end}/tests/r30f_optional_safety_diag_codec_test.cpp").is_file()
boat=read("Boat/source/firmware/Flytec_Boat.ino")
assert "safetyTelemetry.kind=RADIO_PAYLOAD_SAFETY_DIAGNOSTIC;" in boat
assert "radio.sendHardwareTelemetry(safetyTelemetry);" in boat
assert "application.attach(" not in boat
assert "safetyObservations.snapshot(" in boat
radio=read("Boat/source/boat_dual_radio_port.cpp")
# R30G replaced inline optional TX with a bounded one-slot offer. Check the
# session/primary eligibility at the offer, then authenticated primary send
# after heartbeat/command/mission work, not an obsolete exact code line.
assert "if(!sessionReady()||!primaryFresh(now))" in radio
assert "if(status.kind==RADIO_PAYLOAD_SAFETY_DIAGNOSTIC)" in radio
assert "optionalDiagnostics_.offer(payload,now)" in radio
assert "optionalDiagnostics_.take(opportunity,optionalNow,optionalPayload)" in radio
assert "radio_frameSeal(optional,key_)&&sendFrame(LINK_NRF_PRIMARY,optional)" in radio
assert "radio_frameSeal(f,key_)" in radio and "sendFrame(LINK_NRF_PRIMARY,f)" in radio
receiver=read("Remote/source/boat_radio.cpp")
assert "radio_frameVerify(frame,g_authKey)" in receiver
assert "frame.payload[0]==radio_r30f::SUBTYPE ||" in receiver
assert "frame.payload[0]==flytec_time::SUBTYPE" in receiver
assert receiver.index("frame.payload[0]==radio_r30f::SUBTYPE")<receiver.index(
        "g_linkManager.reportValidRx(link,true,now);")
telemetry=read("Remote/source/boat_telemetry.cpp")
assert "case RADIO_PAYLOAD_SAFETY_DIAGNOSTIC:" in telemetry
assert "if(!primary||!radio_r30f::validate(d))return false;" in telemetry
assert "telemetry.diagnosticReceivedAtMs=now;" in telemetry
assert "s.diagnosticFresh=" in telemetry
assert "s.actuatorSafetyReady=statusFresh" in telemetry
# The diagnostic case MUST NOT mark normal STATUS, battery charge, or link
# liveness as received. Code outside this branch remains legacy behavior.
branch=telemetry.split("case RADIO_PAYLOAD_SAFETY_DIAGNOSTIC:",1)[1].split("case RADIO_PAYLOAD_WEATHER:",1)[0]
for forbidden in ("statusReceivedAtMs=", "batteryReceivedAtMs=", "receivedAtMs=now",
                  "actuatorSafetyReady=", "emergencyStopConfirmed=", "reportValidRx("):
    assert forbidden not in branch,forbidden
ui=read("Remote/source/boat_ui.cpp")
assert "diagnosticStatusLabel" in ui and "НЕПОТВ." in ui
assert "diagnosticReasonBulgarian" in ui
assert "RADIO_PROTOCOL_VERSION = 1" in read("Boat/source/radio_protocol.h")
assert read("Boat/source/radio_protocol.h")==read("Remote/source/radio_protocol.h")
assert "ACTUAL_READER_COMMISSIONED = false" in read("Boat/source/boat_common_bus_readonly_port_r30d.h")
assert "COMMON_BUS_HARDWARE_COMMISSIONED = false" in read("Boat/source/boat_common_bus_feedback_r30c.h")
assert "SHARED_BUS_PHYSICALLY_ACCEPTED = false" in read("Boat/source/boat_r29_shared_bus_contract.h")
assert "SEPARATE_MOTOR_SOLENOID_POWER = false" in read("Boat/source/boat_r29_shared_bus_contract.h")
assert "INA231_PHYSICAL_COMMISSIONED=false" in read("Boat/source/ina231_voltage_contract_r27.h")
with (root/"Boat/docs/BOAT_R30A_READINESS_GATES.csv").open(newline="",encoding="utf-8") as f:
    gates=list(csv.DictReader(f))
assert len(gates)==24 and all(g["State"]=="OPEN" for g in gates)
doc=read("Boat/docs/BOAT_R30F_DIAGNOSTIC_WIRE_PROTOCOL.md")
assert "NOT A CONTROL PERMISSION" in doc and "UNVERIFIED" in doc
print("R30F shared v1 subtype 8 identical; authenticated primary-only diagnostic isolation and 24 physical gates PASS")
