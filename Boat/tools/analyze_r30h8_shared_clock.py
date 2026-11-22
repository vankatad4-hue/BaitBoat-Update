#!/usr/bin/env python3
"""R30H-8 read-only PPS/common-instrument clock-offset *candidate* analysis.

This is NOT a PPS detector or an ESP32 firmware clock calibration. A physical
PPS wire and independently audited GPIO UTC-sample markers are NOT installed.
Never accepts USB_LOCAL/ESP millis timestamps as one common instrument clock.
A real-looking manifest cannot grant physical STOP, GPS, time or PPS signoff.

Return codes: 2 invalid/incomplete; 3 NOT_RUN; 4 needs physical review or
synthetic noncertifiable. There is intentionally no exit-0 PASS path.
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

REQUIRED = {
    "capture_id", "trial_id", "clock_domain", "time_us", "platform",
    "event", "utc_unix_ms", "origin",
}
EVENT_PLATFORMS = {
    "GNSS_PPS_EDGE": "GNSS",
    "BOAT_RTC_SAMPLE_EDGE": "BOAT",
    "REMOTE_RTC_SAMPLE_EDGE": "REMOTE",
}
EVENTS = set(EVENT_PLATFORMS)
UNIX_MIN = 1577836800000  # 2020-01-01 UTC
UNIX_MAX = 4102444800000  # 2100-01-01 UTC
MAX_ROWS = 50000


def parse_csv(path):
    trials = defaultdict(dict)
    origins = set()
    clocks = set()
    previous_by_capture = {}
    count = 0
    with path.open("r", newline="", encoding="utf-8-sig") as stream:
        reader = csv.DictReader(stream)
        if not reader.fieldnames or not REQUIRED.issubset(reader.fieldnames):
            raise ValueError("R30H8-1 capture header is incomplete")
        for lineno, row in enumerate(reader, 2):
            if not any((value or "").strip() for value in row.values() if value is not None):
                continue
            if count >= MAX_ROWS:
                raise ValueError("too many capture rows")
            count += 1
            if any(row.get(f) is None for f in REQUIRED):
                raise ValueError(f"line {lineno}: missing required field")
            capture, trial, clock, platform, event, origin = (
                row[key].strip() for key in
                ("capture_id", "trial_id", "clock_domain", "platform", "event", "origin")
            )
            if not capture or not trial or not clock:
                raise ValueError(f"line {lineno}: missing capture/trial/clock")
            if EVENT_PLATFORMS.get(event) != platform:
                raise ValueError(f"line {lineno}: unknown or mismatched event {event}/{platform}")
            if origin not in {"SYNTHETIC", "EXTERNAL_ANALYZER"}:
                raise ValueError(f"line {lineno}: local firmware/USB timestamps are not one physical clock")
            raw_time = row["time_us"].strip()
            raw_utc = row["utc_unix_ms"].strip()
            if not re.fullmatch(r"[0-9]+", raw_time) or not re.fullmatch(r"[0-9]+", raw_utc):
                raise ValueError(f"line {lineno}: timestamp must be unsigned integer")
            time_us, utc_ms = int(raw_time), int(raw_utc)
            if time_us > (1 << 63) - 1 or utc_ms < UNIX_MIN or utc_ms >= UNIX_MAX:
                raise ValueError(f"line {lineno}: invalid monotonic time or UTC range")
            if event == "GNSS_PPS_EDGE" and utc_ms % 1000 != 0:
                raise ValueError(f"line {lineno}: PPS edge must identify a whole UTC second")
            prior = previous_by_capture.get(capture)
            if prior is not None and time_us < prior:
                raise ValueError(f"line {lineno}: capture timestamps out of order")
            previous_by_capture[capture] = time_us
            key = (capture, trial)
            if event in trials[key]:
                raise ValueError(f"line {lineno}: duplicate {event} within one trial")
            trials[key][event] = {
                "time_us": time_us, "utc_ms": utc_ms, "clock": clock,
                "origin": origin, "event": event,
            }
            origins.add(origin)
            clocks.add(clock)
    return trials, origins, clocks


def read_manifest(path):
    with path.open("r", encoding="utf-8") as stream:
        manifest = json.load(stream)
    if manifest.get("schema") != "R30H8-1":
        raise ValueError("invalid manifest schema")
    if manifest.get("evidence_mode") not in (
        "UNSPECIFIED", "SYNTHETIC", "HARDWARE_OBSERVATION"
    ):
        raise ValueError("invalid evidence_mode")
    return manifest


def validate_provenance(manifest, raw_path, trials, origins, clocks):
    if not trials:
        return []
    errors = []
    mode = manifest["evidence_mode"]
    expected = {
        "SYNTHETIC": "SYNTHETIC",
        "HARDWARE_OBSERVATION": "EXTERNAL_ANALYZER",
    }
    if mode == "UNSPECIFIED" or origins != {expected.get(mode)}:
        errors.append("nonempty capture has inconsistent/unspecified origin and evidence_mode")
    if len(clocks) != 1 or manifest.get("clock_domain") not in clocks:
        errors.append("no single externally identified physical clock")
    for entries in trials.values():
        if len({r["clock"] for r in entries.values()}) != 1:
            errors.append("one trial mixes multiple clock domains")
    if mode == "HARDWARE_OBSERVATION":
        if manifest.get("capture_sha256") != hashlib.sha256(raw_path.read_bytes()).hexdigest():
            errors.append("missing or incorrect SHA256 of captured CSV bytes")
        instrument = manifest.get("instrument") or {}
        try:
            sample_rate_hz = float(instrument.get("sample_rate_hz", 0))
        except (ValueError, TypeError):
            sample_rate_hz = 0
        if (not instrument.get("model") or not instrument.get("asset_id")
                or not math.isfinite(sample_rate_hz) or sample_rate_hz <= 0):
            errors.append("missing real external analyzer identity or sample rate")
        fw = manifest.get("firmware") or {}
        for endpoint in ("boat_commit", "remote_commit"):
            if not re.fullmatch(r"[a-f0-9]{40}", str(fw.get(endpoint, ""))):
                errors.append(f"missing full firmware identity {endpoint}")
        gnss = manifest.get("pps") or {}
        if (gnss.get("physical_pps_present") is not True or
                gnss.get("pps_utc_second_association_verified") is not True or
                gnss.get("pps_edge_polarity") not in ("RISING", "FALLING") or
                not gnss.get("gnss_module_revision") or
                not gnss.get("witness_evidence")):
            errors.append("unverified physical GNSS PPS edge or UTC epoch relationship")
        markers = manifest.get("sample_markers") or {}
        if (not markers.get("boat_gpio_pin") or not markers.get("remote_gpio_pin") or
                markers.get("logic_level_verified") is not True or
                markers.get("time_read_and_marker_latency_calibrated") is not True or
                not markers.get("probe_evidence")):
            errors.append("unverified physical clock-sample GPIO timing/voltage")
        hw = manifest.get("hardware") or {}
        if (hw.get("actuator_power_disconnected") is not True or
                hw.get("radio_rf_audited") is not True or
                not hw.get("remote_board_revision") or not hw.get("boat_board_revision")):
            errors.append("RF-only power/board/antenna gates missing")
        if not manifest.get("reviewer"):
            errors.append("independent acquisition reviewer not supplied")
    return errors


def compute(trials):
    """Offset relative to externally time-associated PPS; NOT a measured signoff."""
    errors = []
    results = []
    for (capture, trial), entries in sorted(trials.items()):
        if set(entries) != EVENTS:
            errors.append(f"{capture}/{trial}: PPS + both clock sample edges required")
            continue
        pps = entries["GNSS_PPS_EDGE"]
        boat = entries["BOAT_RTC_SAMPLE_EDGE"]
        remote = entries["REMOTE_RTC_SAMPLE_EDGE"]
        for sample in (boat, remote):
            relative = sample["time_us"] - pps["time_us"]
            # The exact UTC second association belongs ONLY to this PPS period.
            if not (0 <= relative < 1000000):
                errors.append(f"{capture}/{trial}: marker outside anchored PPS second")
        if errors and (errors[-1].startswith(f"{capture}/{trial}")):
            continue
        # Integer microseconds; no loss from 1.7e12-ms UNIX epoch conversion.
        boat_offset_us = 1000 * (boat["utc_ms"] - pps["utc_ms"]) - (
            boat["time_us"] - pps["time_us"]
        )
        remote_offset_us = 1000 * (remote["utc_ms"] - pps["utc_ms"]) - (
            remote["time_us"] - pps["time_us"]
        )
        results.append({
            "capture": capture,
            "trial": trial,
            "clock": pps["clock"],
            "pps_time_us": pps["time_us"],
            "pps_utc_ms": pps["utc_ms"],
            "boat_offset_us": boat_offset_us,
            "remote_offset_us": remote_offset_us,
            "remote_minus_boat_us": remote_offset_us - boat_offset_us,
        })
    # Detect impossible PPS epoch/time order within each capture.
    groups = defaultdict(list)
    for row in results:
        groups[row["capture"]].append(row)
    for capture, rows in groups.items():
        rows.sort(key=lambda row: row["pps_time_us"])
        for prior, current in zip(rows, rows[1:]):
            if current["pps_time_us"] <= prior["pps_time_us"]:
                errors.append(f"{capture}: PPS instrument timestamp not increasing")
            if current["pps_utc_ms"] <= prior["pps_utc_ms"]:
                errors.append(f"{capture}: UTC second stamp reordered/replayed")
    return errors, results


def describe(results):
    if not results:
        return {}
    metrics = {}
    for name in ("boat_offset_us", "remote_offset_us", "remote_minus_boat_us"):
        values = [row[name] for row in results]
        metrics[name] = {
            "samples": len(values),
            "min": min(values),
            "median": statistics.median(values),
            "max": max(values),
        }
    # Drift estimate is observational, not a bound on oscillator calibration.
    drifts = {}
    for endpoint in ("boat", "remote"):
        by_capture = defaultdict(list)
        for row in results:
            by_capture[row["capture"]].append(row)
        for capture, samples in by_capture.items():
            samples.sort(key=lambda s: s["pps_time_us"])
            if len(samples) < 2:
                continue
            elapsed = samples[-1]["pps_time_us"] - samples[0]["pps_time_us"]
            if elapsed <= 0:
                continue
            drifts[f"{capture}/{endpoint}_ppm_candidate"] = (
                (samples[-1][f"{endpoint}_offset_us"] -
                 samples[0][f"{endpoint}_offset_us"]) * 1e6 / elapsed
            )
    return {"offset_candidates_us": metrics, "drift_candidates_ppm": drifts}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_csv", type=pathlib.Path)
    parser.add_argument("manifest_json", type=pathlib.Path)
    args = parser.parse_args(argv)
    try:
        manifest = read_manifest(args.manifest_json)
        trials, origins, clocks = parse_csv(args.capture_csv)
        if not trials:
            print("R30H-8 NOT_RUN: no PPS, Boat clock, or Remote clock edge")
            print("PHYSICAL CLOCK ALIGNMENT: NOT_MEASURED; RF/STOP CURRENT-ZERO: NOT_RUN")
            return 3
        errors = validate_provenance(manifest, args.capture_csv, trials, origins, clocks)
        calculation_errors, results = compute(trials)
        errors += calculation_errors
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print("R30H-8 INVALID:", exc)
        return 2
    if errors:
        for error in errors:
            print("R30H-8 INVALID:", error)
        print("NO CLOCK/PPS/RF PHYSICAL SIGNOFF")
        return 2
    print(json.dumps({
        "result": ("SIMULATED_NONCERTIFIABLE" if manifest["evidence_mode"] == "SYNTHETIC"
                   else "EXTERNAL_CAPTURE_REVIEW_REQUIRED"),
        "trial_count": len(results),
        "numbers_are": "CONDITIONAL_ON_UNVERIFIED_GPIO_EDGE_LATENCY_AND_PPS_EPOCH",
        "analysis": describe(results),
        "physical_clock_alignment": "NOT_SIGNED",
        "physical_stop_current_zero": "NOT_RUN",
    }, sort_keys=True, indent=2))
    return 4


if __name__ == "__main__":
    sys.exit(main())
