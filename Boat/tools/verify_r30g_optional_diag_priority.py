#!/usr/bin/env python3
"""R30G regressions: optional nRF diagnostic never owns STOP/control priority."""
from pathlib import Path
import csv
ROOT=Path(__file__).resolve().parents[2]
def read(p):return (ROOT/p).read_text(encoding="utf-8")
s=read("Boat/source/boat_dual_radio_port.cpp")
h=read("Boat/source/boat_dual_radio_port.h")
budget=read("Boat/source/boat_r30g_optional_diag_budget.h")
for token in (
    "MAX_PENDING_AGE_MS=500U",
    "MIN_DIAGNOSTIC_PERIOD_MS=2000U",
    "MIN_IDLE_AFTER_COMMAND_MS=250U",
    "HEARTBEAT_GAP_BEGIN_MS=15U",
    "HEARTBEAT_GAP_END_MS=40U",
    "txPending", "commandQueued", "activeMission", "recentCommand",
    "failsafeOrLinkLoss", "bool take(", "bool offer(",
    "pending_=false", "discard();return false",
):
    assert token in budget,token
assert "boat_r30g::OptionalDiagnosticBudget optionalDiagnostics_" in h
assert "if(status.kind==RADIO_PAYLOAD_SAFETY_DIAGNOSTIC)" in s
assert "optionalDiagnostics_.offer(payload,now)" in s
assert "if(!sessionReady()||!primaryFresh(now))" in s
assert "if(!radio_encodeTelemetryPayload(status,payload))return false" in s
assert "opportunity.txPending=primaryDriverPending();" in s
assert "bool BoatDualRadioPort::primaryDriverPending() const{return nrf_.txPending();}" in s
assert "opportunity.commandQueued=commandQueue_.size()!=0||stopAck_.pending()||statusOutbox_.pending();" in s
# R30H-19 STOP ACK and pending real STATUS both block optional diagnostics.
assert "opportunity.activeMission=controller_.status().transaction!=0;" in s
assert "opportunity.failsafeOrLinkLoss=linkLossHold_||detachedReturn_||failsafeRthActive_;" in s
assert "opportunity.recentCommand=lastControlRxValid_" in s
assert "opportunity.haveHeartbeat=havePrimaryHeartbeatSent_;" in s
assert "optionalDiagnostics_.take(opportunity,optionalNow,optionalPayload)" in s
assert "optionalDiagnostics_.sent(optionalNow)" in s
assert "optionalDiagnostics_.rejectedTx()" in s
assert "optionalDiagnostics_.reset();lastControlRxValid_=false" in s
assert "havePrimaryHeartbeatSent_=false;" in s
# Optional TX is attempted only AFTER heartbeat, mission and watchdog.
update=s.split("void BoatDualRadioPort::update() {",1)[1].split(
    "bool BoatDualRadioPort::maybeStartFailsafeReturnHome",1)[0]
assert update.index("radioWatchdog_.update(")<update.index(
    "sendMissionFeedback(")<update.index("sendHeartbeat(LINK_NRF_PRIMARY")
assert update.index("sendHeartbeat(LINK_NRF_PRIMARY")<update.index(
    "optionalDiagnostics_.take(")<update.index("sendFrame(LINK_NRF_PRIMARY,optional)")
assert "if(sendHeartbeat(LINK_NRF_PRIMARY,now))" in update
assert "lastNrfHeartbeatTxMs_=now;havePrimaryHeartbeatSent_=true;" in update
assert "lastControlRxValid_=true;lastControlRxMs_=now;" in s
# Existing direct non-diagnostic telemetry, ACK, mission and link policies
# remain separate; diagnostic is never an E22 backup payload.
direct=s.split("bool BoatDualRadioPort::sendHardwareTelemetry(",1)[1].split(
    "bool BoatDualRadioPort::sendMissionFeedback(",1)[0]
assert "return radio_encodeTelemetryPayload(status,f.payload)&&radio_frameSeal(f,key_)&&sendFrame(LINK_NRF_PRIMARY,f);" in direct
assert "sendMissionFeedback(controller_.status()" in update
remote=read("Remote/source/boat_radio.cpp")
assert "frame.payload[0]==radio_r30f::SUBTYPE ||" in remote
assert "frame.payload[0]==flytec_time::SUBTYPE" in remote
# Captured RX path: verify authentication, payload dispatch, informational
# subtype early return and only then link freshness at the original capture.
guard=remote.index("if(!g_session.established()||frame.sessionEpoch!=g_sessionEpoch||!radio_frameVerify(frame,g_authKey))return;")
dispatch=remote.index("if(!dispatchValidatedPayload(link,frame,capturedAt))return;",guard)
skip=remote.index("frame.payload[0]==radio_r30f::SUBTYPE ||",dispatch)
skip_return=remote.index("frame.payload[0]==flytec_time::SUBTYPE))return;",skip)
report=remote.index("g_linkManager.reportValidRx(link,true,capturedAt);",skip_return)
assert guard<dispatch<skip<skip_return<report
tel=read("Remote/source/boat_telemetry.cpp")
assert "if(!primary||!radio_r30f::validate(d))return false;" in tel
assert "s.diagnosticFresh=" in tel
assert "SAFETY_DIAGNOSTIC_MAX_AGE_MS=6000U" in tel
assert "s.actuatorSafetyReady=statusFresh" in tel
assert read("Boat/source/radio_safety_diagnostic_v1.h")==read(
    "Remote/source/radio_safety_diagnostic_v1.h")
for p,token in (
 ("Boat/source/boat_common_bus_feedback_r30c.h","COMMON_BUS_HARDWARE_COMMISSIONED = false"),
 ("Boat/source/boat_common_bus_readonly_port_r30d.h","ACTUAL_READER_COMMISSIONED = false"),
 ("Boat/source/ina231_voltage_contract_r27.h","INA231_PHYSICAL_COMMISSIONED=false"),
 ("Boat/source/boat_r29_shared_bus_contract.h","SHARED_BUS_PHYSICALLY_ACCEPTED = false"),
 ("Boat/source/boat_r29_shared_bus_contract.h","SEPARATE_MOTOR_SOLENOID_POWER = false"),
):
    assert token in read(p),(p,token)
assert "application.attach(" not in read("Boat/source/firmware/Flytec_Boat.ino")
with (ROOT/"Boat/docs/BOAT_R30A_READINESS_GATES.csv").open(newline="",encoding="utf-8") as f:
    gates=list(csv.DictReader(f))
assert len(gates)==24 and all(x["State"]=="OPEN" for x in gates)
assert "NOT" in read("Boat/docs/BOAT_R30G_RADIO_DIAGNOSTIC_PRIORITY.md")
print("R30G optional diagnostic nRF priority, Remote STOP freshness and 24 hardware locks PASS (source audit)")
