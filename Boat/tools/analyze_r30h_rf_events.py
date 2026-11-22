#!/usr/bin/env python3
"""R30H measured-event checker. Empty logs are NOT_RUN, never PASS.

Each trial must use ONE common clock_domain for cross-device timestamps.
This checks event ordering and premature RTH; it does not commission hardware.
"""
import argparse
import csv
import math
import statistics
import sys
from collections import defaultdict

STOP_CASES = {"H04", "H05", "H06", "H07"}
LOSS_CASES = {"H08", "H11", "H12", "H13", "H14", "H15", "H16"}
RTH_CASES = {"H11"}
NO_RTH_CASES = {"H12", "H13", "H14", "H15", "H16", "H17"}
BEFORE_RECOVERY = {"H17"}
SUPPORTED_CASES = STOP_CASES | LOSS_CASES | BEFORE_RECOVERY
ALL_EVENTS = {
    "INPUT_STOP", "TX_STOP_ATTEMPT", "AUTH_STOP_RX", "STOP_INTENT",
    "LOGICAL_OUTPUT_OFF", "DUMMY_CURRENT_ZERO",
    "RF_BLOCKED", "LAST_AUTH_RX", "BOTH_STALE", "RTH_ENTRY",
    "AUTH_LINK_RESTORED", "MISSION_RESUME", "RELEASE_ACTIVE",
}
REQUIRED_COLUMNS = {"trial_id", "case_id", "transport", "clock_domain", "platform", "event", "time_us"}


def parse(path):
    trials = defaultdict(lambda: {"meta": None, "events": {}})
    with open(path, encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        if not reader.fieldnames or not REQUIRED_COLUMNS.issubset(reader.fieldnames):
            raise ValueError("Missing required R30H header columns")
        for lineno, row in enumerate(reader, 2):
            if not row["trial_id"] and not row["event"] and not row["time_us"]:
                continue
            trial = row["trial_id"].strip()
            case = row["case_id"].strip()
            event = row["event"].strip()
            clock = row["clock_domain"].strip()
            if not trial or not case or not clock or event not in ALL_EVENTS:
                raise ValueError(f"line {lineno}: missing trial/case/clock or unknown event")
            try:
                micros = int(row["time_us"].strip())
            except ValueError as exc:
                raise ValueError(f"line {lineno}: time_us must be an integer") from exc
            if micros < 0:
                raise ValueError(f"line {lineno}: negative timestamp")
            key = trials[trial]
            meta = (case, row["transport"].strip(), clock)
            if key["meta"] is None:
                key["meta"] = meta
            elif key["meta"] != meta:
                raise ValueError(f"line {lineno}: trial spans different case/transport/clock domain")
            if event in key["events"]:
                raise ValueError(f"line {lineno}: duplicate event {event} in {trial}; use separate trial IDs")
            key["events"][event] = micros
    return trials


def check(trial, record, stop_limit_ms=None, loss_limit_ms=None):
    case, transport, clock = record["meta"]
    e = record["events"]
    errors, metrics = [], {}
    def elapsed(label, start, end):
        if start not in e or end not in e:
            errors.append(f"{label}: missing {start} or {end}")
            return
        diff = e[end] - e[start]
        if diff < 0:
            errors.append(f"{label}: reversed event timestamps")
        else:
            metrics[label] = diff / 1000.0
    if case in STOP_CASES:
        elapsed("stop_transport_ms", "INPUT_STOP", "AUTH_STOP_RX")
        elapsed("boat_stop_processing_ms", "AUTH_STOP_RX", "STOP_INTENT")
        elapsed("stop_e2e_logic_ms", "INPUT_STOP", "LOGICAL_OUTPUT_OFF")
        if "DUMMY_CURRENT_ZERO" in e:
            elapsed("stop_e2e_current_ms", "INPUT_STOP", "DUMMY_CURRENT_ZERO")
        if stop_limit_ms is not None and metrics.get("stop_e2e_logic_ms", float("inf")) > stop_limit_ms:
            errors.append("STOP exceeds configured measured limit")
    if case in LOSS_CASES:
        elapsed("rf_cut_to_stale_ms", "RF_BLOCKED", "BOTH_STALE")
        elapsed("stale_to_logic_off_ms", "BOTH_STALE", "LOGICAL_OUTPUT_OFF")
        elapsed("rf_cut_to_logic_off_ms", "RF_BLOCKED", "LOGICAL_OUTPUT_OFF")
        if "LAST_AUTH_RX" not in e:
            errors.append("missing LAST_AUTH_RX")
        elif "BOTH_STALE" in e and e["BOTH_STALE"] < e["LAST_AUTH_RX"]:
            errors.append("BOTH_STALE preceded LAST_AUTH_RX")
        if loss_limit_ms is not None and metrics.get("rf_cut_to_logic_off_ms", float("inf")) > loss_limit_ms:
            errors.append("loss-to-stop exceeds configured measured limit")
    if case in RTH_CASES:
        elapsed("last_auth_to_rth_ms", "LAST_AUTH_RX", "RTH_ENTRY")
        if metrics.get("last_auth_to_rth_ms", -1) < 10000.0:
            errors.append("RTH entry missing or EARLY (<10000ms from last accepted authenticated RX)")
        if "BOTH_STALE" in e and "RTH_ENTRY" in e and e["RTH_ENTRY"] < e["BOTH_STALE"]:
            errors.append("RTH before BOTH_STALE")
    if case in NO_RTH_CASES and "RTH_ENTRY" in e:
        errors.append("forbidden RTH entry for this scenario")
    if case in BEFORE_RECOVERY:
        elapsed("rf_cut_to_restore_ms", "RF_BLOCKED", "AUTH_LINK_RESTORED")
        if "LAST_AUTH_RX" not in e or "AUTH_LINK_RESTORED" not in e:
            errors.append("missing last RX or restoration")
        elif (e["AUTH_LINK_RESTORED"] - e["LAST_AUTH_RX"]) >= 10000000:
            errors.append("not a recovery BEFORE 10 seconds")
    if case in RTH_CASES | NO_RTH_CASES and "RELEASE_ACTIVE" in e:
        errors.append("release active during loss/RTH")
    # Unmodeled cases require human signoff, never an automated success label.
    assessed = case in SUPPORTED_CASES
    # An event-only log is not independent evidence of physical current zero.
    return errors, metrics, case, transport, clock, assessed


def summarize(values):
    sorted_values = sorted(values)
    p95 = sorted_values[max(0, math.ceil(0.95 * len(sorted_values)) - 1)]
    return (min(values), statistics.median(values), p95, max(values))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv_file", help="Measured R30H event CSV; one common clock per trial")
    parser.add_argument("--stop-limit-ms", type=float, default=None, help="Signed limit; not a default certification")
    parser.add_argument("--loss-stop-limit-ms", type=float, default=None, help="Signed total RF-cut to stop limit")
    args = parser.parse_args()
    if any(x is not None and (not math.isfinite(x) or x <= 0) for x in (args.stop_limit_ms, args.loss_stop_limit_ms)):
        parser.error("latency limits must be finite and positive")
    try:
        trials = parse(args.csv_file)
    except (OSError, ValueError) as exc:
        print(f"INVALID_LOG: {exc}")
        return 2
    if not trials:
        print("NOT_RUN: empty evidence CSV; NO physical acceptance")
        return 3
    totals, failure, human_review = defaultdict(list), False, False
    for trial, record in sorted(trials.items()):
        errors, metrics, case, transport, clock, assessed = check(trial, record, args.stop_limit_ms, args.loss_stop_limit_ms)
        failure |= bool(errors)
        human_review |= not assessed
        status = 'FAIL' if errors else ('EVENT_CHECK_OK' if assessed else 'MANUAL_REVIEW_REQUIRED')
        print(f"{trial} {case} {transport} clock={clock}: {status}")
        for label, value in sorted(metrics.items()):
            print(f"  {label}={value:.3f}ms")
            totals[(case, label)].append(value)
        for error in errors:
            print(f"  ERROR: {error}")
    print("Measured metrics by scenario (min/median/p95/max ms; not physical signoff):")
    for (case, label), values in sorted(totals.items()):
        mi, med, p95, ma = summarize(values)
        print(f"  {case} {label} n={len(values)}: {mi:.3f}/{med:.3f}/{p95:.3f}/{ma:.3f}")
    print("RESULT:", "FAIL" if failure else ("MANUAL_REVIEW_REQUIRED - unsupported scenarios remain unsigned" if human_review else "EVENTS_CONSISTENT_ONLY - hardware gates and sample counts NOT assessed"))
    return 2 if failure else (4 if human_review else 0)


if __name__ == "__main__":
    sys.exit(main())
