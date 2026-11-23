#!/usr/bin/env python3
"""R30H-1 static preflight: enforce UNCOMMISSIONED real-radio + SC16 TXLVL semantics.

Source-consistency check only. A PASS does NOT prove RF pairing, range,
power integrity, driver current-zero or physical STOP. Never sign gates in CI.
"""
import csv
import io
import json
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
    "Boat/source/tracker_gnss.h",
    "Boat/source/gnss_utc_sync.h",
    "Boat/source/radio_utc_time_v1.h",
    "Remote/source/radio_utc_time_v1.h",
    "Boat/source/r30h6_utc_marker_policy.h",
    "Remote/source/r30h6_utc_marker_policy.h",
    "Boat/docs/BOAT_R30H_1_RF_MODULE_SIGNOFF.csv",
    "Boat/docs/BOAT_R30H_RF_TEST_MATRIX.csv",
    "Boat/docs/BOAT_R30H_3_STOP_RF_ACCEPTANCE.csv",
    "Boat/docs/BOAT_R30H_4_LOGIC_CAPTURE_TEMPLATE.csv",
    "Boat/docs/BOAT_R30H_4_CAPTURE_MANIFEST_TEMPLATE.json",
    "Boat/docs/BOAT_R30H_4_RF_ONLY_PROBE_CROSSWALK.csv",
    "Boat/docs/BOAT_R30H_7_GNSS_PPS_RF_CLOCK_SIGNOFF.csv",
    "Boat/docs/BOAT_R30H_8_SHARED_CLOCK_CAPTURE_TEMPLATE.csv",
    "Boat/docs/BOAT_R30H_8_SHARED_CLOCK_MANIFEST_TEMPLATE.json",
    "Boat/docs/BOAT_R30H_8_CLOCK_ALIGNMENT_GATES.csv",
)
LOCKED_MACROS = {
    "Remote/source/radio_hw_config.h": (
        "REMOTE_NRF_PIN_AUDIT_SIGNED",
        "REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED",
        "REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED",
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
    need(remote, "g_nrfReady=REMOTE_NRF_PIN_AUDIT_SIGNED &&")
    need(remote, "REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED &&")
    need(remote, "REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED &&")
    need(remote, "g_nrf.begin(true)")
    need(remote, "Sc18is606NrfLink g_nrf(")
    need(remote, "g_linkManager.setTransportReady(LINK_NRF_PRIMARY,false)")
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
    # R30H-7: GPS UTC must never become STOP/RTH or a fabricated
    # physical common-clock reference; GNSS date must match position epoch.
    need("Boat/source/tracker_gnss.h", "positionUtc_!=rmcUtc_||\n     (uint32_t)(now-gsaAt_)")
    need("Remote/source/boat_radio.cpp",
         "g_linkManager.primaryHealthy(at)&&")
    need("Boat/source/firmware/Flytec_Boat.ino", "settimeofday(&tv,nullptr)")
    need("Remote/source/boat_radio.cpp", "settimeofday(&tv,nullptr)")
    need("Remote/source/boat_radio.cpp",
         "frame.payload[0]==flytec_time::SUBTYPE))return;")
    if source["Boat/source/radio_utc_time_v1.h"] != source["Remote/source/radio_utc_time_v1.h"]:
        errors.append("Boat/Remote GPS UTC wire discipline headers differ")
    if source["Boat/source/r30h6_utc_marker_policy.h"] != source["Remote/source/r30h6_utc_marker_policy.h"]:
        errors.append("Boat/Remote debug marker gate headers differ")
    for p in ("Boat/source/r30h6_utc_marker_policy.h","Remote/source/r30h6_utc_marker_policy.h"):
        need(p, "bool rfEndToEndStopClock=false;")
        need(p, "bool traceReferenceCertified=false;")
    if "settimeofday" in source[boat] or "settimeofday" in source[remote].split("bool boatRadio_utcNow")[1]:
        errors.append("UTC clock adjustment must stay outside Boat safety radio loop and Remote public UTC accessor")
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
    # R30H-4: do not promote unmeasured analyzer edges, probe mappings, or
    # self-declared metadata to approved RF/STOP timing in the repository.
    capture = "Boat/docs/BOAT_R30H_4_LOGIC_CAPTURE_TEMPLATE.csv"
    reader = csv.DictReader(io.StringIO(source[capture]))
    if not reader.fieldnames or not {"capture_id","trial_id","case_id","clock_domain",
                                      "time_us","platform","event","edge","origin"}.issubset(reader.fieldnames):
        errors.append("R30H-4 external analyzer template schema was altered")
    if list(reader):
        errors.append("R30H-4 authoritative capture must remain empty NOT_RUN")
    manifest_name = "Boat/docs/BOAT_R30H_4_CAPTURE_MANIFEST_TEMPLATE.json"
    try:
        manifest=json.loads(source[manifest_name])
        if manifest.get("schema")!="R30H4-1" or manifest.get("evidence_mode")!="UNSPECIFIED" or manifest.get("capture_file_sha256"):
            errors.append("R30H-4 template cannot claim real/simulated capture or evidence")
    except (ValueError,TypeError,AttributeError):
        errors.append("R30H-4 capture manifest must be valid unsigned JSON")
    probes = list(csv.DictReader(io.StringIO(source["Boat/docs/BOAT_R30H_4_RF_ONLY_PROBE_CROSSWALK.csv"])))
    if len(probes)!=10 or len({r.get("Channel_ID") for r in probes})!=10:
        errors.append("R30H-4 physical crosswalk must list 10 unique unsigned probes")
    for probe in probes:
        if probe.get("Physical_mapping_state")!="NOT_READY" or any(probe.get(field) for field in
             ("Verified_testpoint","Measured_logic_high_V","Measured_logic_low_V","Evidence","Engineer_signoff")):
            errors.append("R30H-4 probe cannot be signed without genuine physical inspection")
            break
    h7 = list(csv.DictReader(io.StringIO(source["Boat/docs/BOAT_R30H_7_GNSS_PPS_RF_CLOCK_SIGNOFF.csv"])))
    if len(h7)!=12 or len({row.get("Gate_ID") for row in h7})!=12:
        errors.append("R30H-7 must keep twelve distinct GNSS/PPS physical acceptance gates")
    for row in h7:
        if row.get("State")!="NOT_RUN" or any(row.get(k) for k in
           ("Observed","Artifact","Engineer_signoff")):
            errors.append("R30H-7 cannot claim physical GPS/PPS/clock-offset measurements")
            break
    # R30H-8: source repo contains only blank lab templates and unsigned
    # physical PPS / dual-clock calibration gates. Signed evidence is a
    # separately reviewed capture, never generated or accepted in CI.
    r8_name="Boat/docs/BOAT_R30H_8_SHARED_CLOCK_CAPTURE_TEMPLATE.csv"
    r8_reader=csv.DictReader(io.StringIO(source[r8_name]))
    if not r8_reader.fieldnames or not {
        "capture_id","trial_id","clock_domain","time_us","platform",
        "event","utc_unix_ms","origin"}.issubset(r8_reader.fieldnames):
        errors.append("R30H-8 PPS/UTC candidate template schema changed")
    if list(r8_reader):
        errors.append("R30H-8 committed PPS evidence template cannot contain physical/synthetic records")
    try:
        r8_manifest=json.loads(source["Boat/docs/BOAT_R30H_8_SHARED_CLOCK_MANIFEST_TEMPLATE.json"])
        if (r8_manifest.get("schema")!="R30H8-1" or
            r8_manifest.get("evidence_mode")!="UNSPECIFIED" or
            r8_manifest.get("capture_sha256") or
            (r8_manifest.get("pps") or {}).get("physical_pps_present") is not None):
            errors.append("R30H-8 unsigned PPS capture manifest cannot be preapproved")
    except (ValueError, TypeError, AttributeError):
        errors.append("R30H-8 manifest must be valid unsigned JSON")
    gates8=list(csv.DictReader(io.StringIO(source["Boat/docs/BOAT_R30H_8_CLOCK_ALIGNMENT_GATES.csv"])))
    if len(gates8)!=8 or len({g.get("Gate_ID") for g in gates8})!=8:
        errors.append("R30H-8 must retain eight distinct physical PPS/clock gates")
    for gate8 in gates8:
        if gate8.get("Status")!="NOT_RUN" or any(gate8.get(k) for k in
            ("Observation","Evidence","Reviewer")):
            errors.append("R30H-8 physical clock/marker acceptance cannot be fabricated")
            break
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
