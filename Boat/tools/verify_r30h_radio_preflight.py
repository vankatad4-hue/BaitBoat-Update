#!/usr/bin/env python3
"""R30H-1 static preflight: enforce UNCOMMISSIONED real-radio + SC16 TXLVL semantics.

Source-consistency check only. A PASS does NOT prove RF pairing, range,
power integrity, driver current-zero or physical STOP. Never sign gates in CI.
"""
import csv
import io
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[2]
REQUIRED = (
    "Remote/source/sc16is750_bridge.cpp",
    "Remote/source/sc16is750_fifo_budget_r30h.h",
    "Remote/source/boat_radio.cpp",
    "Remote/source/boat_radio.h",
    "Remote/source/r30h3_stop_trace.h",
    "Remote/source/radio_hw_config.h",
    "Remote/source/e22_uart_link.cpp",
    "Remote/source/e22_uart_link.h",
    "Boat/source/boat_radio_hw_config.h",
    "Boat/source/boat_dual_radio_port.cpp",
    "Boat/source/boat_dual_radio_port.h",
    "Boat/source/boat_r30h2_local_trace.h",
    "Boat/source/e22_uart_link.cpp",
    "Boat/source/e22_uart_link.h",
    "Boat/source/firmware/Flytec_Boat.ino",
    "Boat/docs/BOAT_R30H_1_RF_MODULE_SIGNOFF.csv",
    "Boat/docs/BOAT_R30H_RF_TEST_MATRIX.csv",
    "Boat/docs/BOAT_R30H_3_STOP_RF_ACCEPTANCE.csv",
)
LOCKED_MACROS = {
    "Remote/source/radio_hw_config.h": (
        "REMOTE_NRF_PIN_AUDIT_SIGNED",
        "REMOTE_E22_BRIDGE_AUDIT_SIGNED",
        "REMOTE_E22_RF_INTEROPERABILITY_VERIFIED",
        "REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED",
        "REMOTE_R30H3_STOP_TRACE_ENABLED",
    ),
    "Boat/source/boat_radio_hw_config.h": (
        "BOAT_NRF_PIN_AUDIT_SIGNED",
        "BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED",
        "BOAT_E22_UART_LOGIC_LEVELS_VERIFIED",
        "BOAT_E22_UART_PIN_AUDIT_SIGNED",
        "BOAT_E22_RF_AIRTIME_AUDIT_SIGNED",
        "BOAT_R30H_LOCAL_TRACE_ENABLED",
    ),
}

def snapshot():
    return {name: (ROOT / name).read_text(encoding="utf-8") for name in REQUIRED}

def verify(source):
    errors = []
    for name in REQUIRED:
        if name not in source:
            errors.append(f"missing file {name}")
    if errors:
        return errors

    def need(name, phrase):
        if phrase not in source[name]:
            errors.append(f"{name}: missing {phrase!r}")

    b = "Remote/source/sc16is750_bridge.cpp"
    h = "Remote/source/sc16is750_fifo_budget_r30h.h"
    need(b, "freeBytes=r30h_sc16::freeTxSlots(level)")
    need(b, "r30h_sc16::txFifoEmpty(level)")
    need(b, '#include "sc16is750_fifo_budget_r30h.h"')
    if "size_t freeBytes=64U-level" in source[b] or "level==0) break" in source[b]:
        errors.append("SC16IS750 inverted TXLVL regression")
    need(h, "txlvl <= FIFO_CAPACITY ? txlvl : 0")
    need(h, "return txlvl == FIFO_CAPACITY;")

    for name, flags in LOCKED_MACROS.items():
        for flag in flags:
            if not re.search(r"^#define\s+" + re.escape(flag) + r"\s+0\s*$", source[name], re.M):
                errors.append(f"{name}: {flag} must remain unsigned 0")
    remote = "Remote/source/boat_radio.cpp"
    boat = "Boat/source/boat_dual_radio_port.cpp"
    need(remote, "g_nrfReady=REMOTE_NRF_PIN_AUDIT_SIGNED && g_nrf.begin(true)")
    for word in ("REMOTE_E22_BRIDGE_AUDIT_SIGNED &&",
                 "REMOTE_E22_RF_INTEROPERABILITY_VERIFIED &&",
                 "REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED &&"):
        need(remote, word)
    need(boat, "&& BOAT_E22_RF_AIRTIME_AUDIT_SIGNED")
    need(remote, "remote_r30h3::E22_PACING_DEFERRED")
    need(remote, "remote_r30h3::E22_UART_BUFFER_ADMITTED")
    need(remote, "remote_r30h3::STOP_BUTTON_REQUEST")
    need("Remote/source/boat_radio.h",
         "boatRadio_takeR30H3StopTrace(remote_r30h3::Record &out)")
    need("Remote/source/r30h3_stop_trace.h",
         "static constexpr uint8_t CAPACITY=16")
    if "Serial." in source[remote]:
        errors.append("Remote R30H-3 must not print in the critical radio/control path")
    need("Boat/source/firmware/Flytec_Boat.ino",
         "static_assert(!boat_r30d::ACTUAL_READER_COMMISSIONED")
    need("Boat/source/firmware/Flytec_Boat.ino",
         "R30H2,BOAT_LOCAL,")
    need("Boat/source/boat_dual_radio_port.h",
         "bool takeR30HTrace(boat_r30h2::Record &out)")
    need("Boat/source/boat_r30h2_local_trace.h",
         "static constexpr uint8_t CAPACITY=32")
    need("Boat/source/boat_dual_radio_port.cpp",
         "boat_r30h2::STOP_AUTH_RECEIVED")
    need("Boat/source/boat_dual_radio_port.cpp",
         "boat_r30h2::BOTH_LINKS_STALE")
    need("Boat/source/boat_dual_radio_port.cpp",
         "boat_r30h2::RTH_LOCAL_ENTRY")
    if "Serial." in source["Boat/source/boat_dual_radio_port.cpp"]:
        errors.append("R30H-2 Serial I/O may not enter radio interrupt/safety path")
    if "application.attach(" in source["Boat/source/firmware/Flytec_Boat.ino"]:
        errors.append("R30H-1 must never attach a real BoatPhysicalDriver")

    def code_bytes(text, field):
        m = re.search(r"\b" + field + r"\[12\]\s*=\s*\{([^}]+)\}", text)
        if not m:
            return None
        try:
            return [int(v.strip(), 0) for v in m.group(1).split(",")]
        except ValueError:
            return None
    boat_e22 = source["Boat/source/e22_uart_link.cpp"]
    remote_e22 = source["Remote/source/e22_uart_link.cpp"]
    for key in ("E22_WRITE_CMD", "E22_WRITE_ACK"):
        a = code_bytes(boat_e22, key)
        b2 = code_bytes(remote_e22, key)
        if not a or len(a) != 12 or a != b2:
            errors.append(f"Boat/Remote E22 {key} differs or is malformed")
        elif a[1:3] != [0, 9] or a[6:9] != [0x67, 0xC3, 0x12]:
            errors.append(f"E22 {key} provisional 9600/62.5k/868.125 profile drift")
    for code in (source["Boat/source/e22_uart_link.h"], source["Remote/source/e22_uart_link.h"]):
        if "EST_AIRTIME_MS=25" not in code:
            errors.append("E22 provisional airtime constant changed; re-audit regulatory budget")
    worksheet = list(csv.DictReader(io.StringIO(source["Boat/docs/BOAT_R30H_1_RF_MODULE_SIGNOFF.csv"])))
    if len(worksheet) != 22 or len({r["Gate_ID"] for r in worksheet}) != 22:
        errors.append("R30H-1 physical signoff must retain 22 unique items")
    for r in worksheet:
        if r.get("State") != "NOT_RUN" or r.get("Observed_or_measured") or r.get("Evidence_path") or r.get("Signoff_by"):
            errors.append(f"R30H-1 physical evidence was altered without physical approval: {r['Gate_ID']}")
    h3 = list(csv.DictReader(io.StringIO(source["Boat/docs/BOAT_R30H_3_STOP_RF_ACCEPTANCE.csv"])))
    if len(h3) != 16 or len({r["Gate_ID"] for r in h3}) != 16:
        errors.append("R30H-3 STOP physical acceptance must retain 16 distinct gates")
    for row in h3:
        if row.get("Status") != "NOT_RUN" or any(row.get(c) for c in (
            "Measured_max_ms", "Approved_limit_ms", "Evidence_link", "Engineering_signoff")):
            errors.append(f"R30H-3 gate cannot be pre-approved: {row['Gate_ID']}")
    matrix = list(csv.DictReader(io.StringIO(source["Boat/docs/BOAT_R30H_RF_TEST_MATRIX.csv"])))
    if len(matrix) != 22 or any(r["State"] != "NOT_RUN" for r in matrix):
        errors.append("R30H physical test matrix must retain 22 NOT_RUN scenarios")
    return errors


def main():
    errors = verify(snapshot())
    if errors:
        for e in errors:
            print("R30H-1 FAIL:", e)
        return 1
    print("R30H-1 source/unsigned preflight PASS; 22 physical signoffs + 22 RF trials remain NOT_RUN")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
