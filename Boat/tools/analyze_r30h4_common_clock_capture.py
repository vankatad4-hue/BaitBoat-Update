#!/usr/bin/env python3
"""R30H-4 external single-clock RF-only acquisition checker.

NEVER awards physical STOP, RF delivery, authenticated command, or release
safety PASS. Hardware-capture mode demands a bound SHA256 manifest and
still returns REVIEW_REQUIRED (exit 4). Synthetic fixtures are also exit 4.
Empty evidence is NOT_RUN (exit 3); malformed evidence exits 2.

CSV event names refer to externally *observed electrical edges*, not packet
authentication, radio completion, logical outputs or motor-current evidence.
"""
import argparse
import csv
import hashlib
import json
import math
import pathlib
import re
import statistics
import sys
from collections import defaultdict

REQUIRED = {"capture_id", "trial_id", "case_id", "clock_domain", "time_us",
            "platform", "event", "edge", "origin"}
ALLOWED_EVENTS = {
    "STOP_BUTTON_EDGE": ("REMOTE", "RISING"),
    "REMOTE_NRF_SPI_CSN": ("REMOTE", "FALLING"),
    "BOAT_NRF_IRQ": ("BOAT", "FALLING"),
    "REMOTE_E22_UART_TX_START": ("REMOTE", "FALLING"),
    "BOAT_E22_UART_RX_START": ("BOAT", "FALLING"),
    "RF_CUT_TRIGGER": ("RF_FIXTURE", "RISING"),
    "BOAT_STOP_INTENT_MARKER": ("BOAT", "RISING"),
    "BOAT_LOGICAL_OUTPUT_OFF_MARKER": ("BOAT", "RISING"),
    "BOAT_RTH_ENTRY_MARKER": ("BOAT", "RISING"),
    "BOAT_LAST_AUTH_RX_MARKER": ("BOAT", "RISING"),
}
PAIR_EVENTS = {
    "H04": [("STOP_BUTTON_EDGE", "REMOTE_NRF_SPI_CSN", "button_to_remote_nrf_spi_ms"),
            ("REMOTE_NRF_SPI_CSN", "BOAT_NRF_IRQ", "nrf_spi_to_boat_irq_ms")],
    "H05": [("STOP_BUTTON_EDGE", "REMOTE_E22_UART_TX_START", "button_to_remote_e22_uart_ms"),
            ("REMOTE_E22_UART_TX_START", "BOAT_E22_UART_RX_START", "e22_uart_tx_to_rx_ms")],
    "H06": [("STOP_BUTTON_EDGE", "BOAT_STOP_INTENT_MARKER", "button_to_boat_intent_ms")],
    "H08": [("RF_CUT_TRIGGER", "BOAT_STOP_INTENT_MARKER", "rf_cut_to_boat_intent_ms")],
    "H11": [("BOAT_LAST_AUTH_RX_MARKER", "BOAT_RTH_ENTRY_MARKER", "last_auth_marker_to_rth_marker_ms")],
}
REQUIRED_START = {"H04": "STOP_BUTTON_EDGE", "H05": "STOP_BUTTON_EDGE",
                  "H06": "STOP_BUTTON_EDGE", "H08": "RF_CUT_TRIGGER",
                  "H11": "BOAT_LAST_AUTH_RX_MARKER"}

def parse_csv(path):
    result = defaultdict(list)
    capture_meta = {}
    with open(path, encoding="utf-8-sig", newline="") as stream:
        reader = csv.DictReader(stream)
        if not reader.fieldnames or not REQUIRED.issubset(reader.fieldnames):
            raise ValueError("missing required R30H-4 capture header")
        previous_us = {}
        for lineno, row in enumerate(reader, 2):
            if not any((v or "").strip() for v in row.values() if v is not None):
                continue
            if any(row.get(field) is None for field in REQUIRED):
                raise ValueError(f"line {lineno}: missing cell")
            capture, trial, case, clock = [row[k].strip() for k in
                                           ("capture_id", "trial_id", "case_id", "clock_domain")]
            platform, event, edge, origin = [row[k].strip() for k in
                                             ("platform", "event", "edge", "origin")]
            if not capture or not trial or not case or not clock:
                raise ValueError(f"line {lineno}: missing capture/trial/case/clock")
            if (platform, edge) != ALLOWED_EVENTS.get(event):
                raise ValueError(f"line {lineno}: invalid event/platform/edge: {event}")
            if origin not in {"EXTERNAL_ANALYZER", "SYNTHETIC"}:
                raise ValueError(f"line {lineno}: USB_LOCAL or unknown origin cannot establish E2E clock")
            raw_ts = row["time_us"].strip()
            if not re.fullmatch(r"[0-9]+", raw_ts):
                raise ValueError(f"line {lineno}: invalid integral external microseconds")
            ts = int(raw_ts)
            if ts > 2**63 - 1:
                raise ValueError(f"line {lineno}: timestamp overflow")
            old = previous_us.get(capture)
            if old is not None and ts < old:
                raise ValueError(f"line {lineno}: capture timestamps not monotonically ordered")
            previous_us[capture] = ts
            if capture in capture_meta and capture_meta[capture] != clock:
                raise ValueError(f"line {lineno}: mixed clock_domain within one capture")
            capture_meta[capture] = clock
            key = (capture, trial)
            if result[key] and (result[key][0]["case"], result[key][0]["clock"]) != (case, clock):
                raise ValueError(f"line {lineno}: trial mixes cases or clocks")
            if any(item["event"] == event for item in result[key]):
                raise ValueError(f"line {lineno}: duplicate event {event} in trial")
            result[key].append({"case": case, "clock": clock, "event": event,
                                "time_us": ts, "origin": origin})
    return result

def read_manifest(path):
    with open(path, encoding="utf-8") as stream:
        manifest = json.load(stream)
    if manifest.get("schema") != "R30H4-1":
        raise ValueError("invalid or missing manifest schema R30H4-1")
    if manifest.get("evidence_mode") not in ("UNSPECIFIED", "SYNTHETIC", "HARDWARE_OBSERVATION"):
        raise ValueError("invalid evidence_mode")
    return manifest

def validate_manifest(manifest, path, trials):
    errors = []
    mode = manifest["evidence_mode"]
    if not trials:
        return errors  # empty template is NOT_RUN
    if mode == "UNSPECIFIED":
        return ["nonempty capture must explicitly declare SYNTHETIC or HARDWARE_OBSERVATION"]
    modes = {r["origin"] for trial in trials.values() for r in trial}
    expected = "EXTERNAL_ANALYZER" if mode == "HARDWARE_OBSERVATION" else "SYNTHETIC"
    if modes != {expected}:
        errors.append("capture provenance mixed or does not match evidence_mode")
    clocks = {r["clock"] for trial in trials.values() for r in trial}
    if len(clocks) != 1 or manifest.get("clock_domain") not in clocks:
        errors.append("missing independent ONE common clock")
    if mode == "HARDWARE_OBSERVATION":
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        if manifest.get("capture_file_sha256") != digest:
            errors.append("raw capture SHA256 missing/mismatched")
        instrument = manifest.get("instrument") or {}
        hw = manifest.get("hardware") or {}
        fw = manifest.get("firmware") or {}
        if not instrument.get("model") or not instrument.get("asset_id"):
            errors.append("missing physical instrument identification")
        try:
            rate = float(instrument.get("sample_rate_hz", 0))
        except (TypeError, ValueError):
            rate = 0
        if not math.isfinite(rate) or rate <= 0:
            errors.append("missing measured analyzer sample rate")
        if hw.get("actuator_power_disconnected") is not True:
            errors.append("RF-only physical capture requires VM/VH physically disconnected")
        for field in ("remote_board_revision", "boat_board_revision",
                      "e22_remote_sku", "e22_boat_sku"):
            if not hw.get(field):
                errors.append(f"missing inspected hardware {field}")
        for field in ("remote_commit", "boat_commit"):
            if not re.fullmatch(r"[a-f0-9]{40}", str(fw.get(field, ""))):
                errors.append(f"missing 40-hex firmware commit {field}")
        if not manifest.get("physical_preflight_evidence"):
            errors.append("missing independently reviewed power/antenna/regulatory evidence reference")
    return errors

def evaluate(trials):
    errors = []
    results = defaultdict(list)
    limited = 0
    for (capture, trial), values in sorted(trials.items()):
        case = values[0]["case"]
        stamps = {r["event"]: r["time_us"] for r in values}
        for start, end, metric in PAIR_EVENTS.get(case, []):
            if start not in stamps or end not in stamps:
                limited += 1
                continue  # Partial observable path, not evidence of failure or STOP success
            delay_us = stamps[end] - stamps[start]
            if delay_us < 0:
                errors.append(f"{capture}/{trial}: {metric} reversed")
            else:
                results[(case, metric)].append(delay_us / 1000)
        required = REQUIRED_START.get(case)
        if required and required not in stamps:
            errors.append(f"{capture}/{trial}: missing primary stimulus {required}")
        if case == "H11" and "BOAT_RTH_ENTRY_MARKER" in stamps and required in stamps:
            if stamps["BOAT_RTH_ENTRY_MARKER"] - stamps[required] < 10_000_000:
                errors.append(f"{capture}/{trial}: RTH marker earlier than 10s from last-auth marker")
        if case.startswith("H") and case not in PAIR_EVENTS:
            limited += 1
    return errors, results, limited

def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("capture_csv", type=pathlib.Path)
    p.add_argument("manifest_json", type=pathlib.Path)
    args = p.parse_args(argv)
    try:
        trials = parse_csv(args.capture_csv)
        manifest = read_manifest(args.manifest_json)
        errors = validate_manifest(manifest, args.capture_csv, trials)
        metrics_err, metrics, limited = evaluate(trials)
        errors.extend(metrics_err)
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print("INVALID_CAPTURE:", exc)
        return 2
    if not trials:
        print("NOT_RUN: no external logic-analyzer edges; all physical gates remain open")
        return 3
    for err in errors:
        print("INVALID_CAPTURE:", err)
    if errors:
        print("NO SIGNOFF; capture does not satisfy minimum evidence provenance/order")
        return 2
    for (case, metric), values in sorted(metrics.items()):
        ordered = sorted(values)
        p95 = ordered[math.ceil(0.95 * len(ordered))-1]
        print(f"{case} {metric} n={len(values)} min/median/p95/max ms="
              f"{ordered[0]:.3f}/{statistics.median(ordered):.3f}/{p95:.3f}/{ordered[-1]:.3f}")
    print("NOTE: SPI CSN, IRQ, UART and marker edges alone do NOT prove authenticated STOP,")
    print("      actual RF delivery, independent DC cutoff, or motor/solenoid current-zero.")
    print(f"mode={manifest['evidence_mode']} trials={len(trials)} partial_or_unmodeled={limited}")
    print("RESULT: SIMULATED_NONCERTIFIABLE" if manifest["evidence_mode"]=="SYNTHETIC" else
          "RESULT: EXTERNAL_CAPTURE_REVIEW_REQUIRED")
    print("PHYSICAL_STOP_SIGNOFF: NOT_RUN; mandatory engineer/witness review and limits OPEN")
    return 4  # Intentionally never PASS or exit 0

if __name__ == "__main__":
    sys.exit(main())
