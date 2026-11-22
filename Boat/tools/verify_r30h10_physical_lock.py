#!/usr/bin/env python3
"""R30H-10 source-only fail-closed gate: never substitute this for physical acceptance.

Checks unsigned physical evidence, primary PlatformIO compile-only settings and
non-actuating GPIO/marker defaults. Future real commissioning requires an
explicit reviewed change, not silently editing a CSV status.
"""
from __future__ import annotations

import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("R30H-10 FAIL CLOSED: " + message)


def text(relpath: str) -> str:
    return (ROOT / relpath).read_text(encoding="utf-8")


def assert_line(relpath: str, exact: str) -> None:
    require(exact in text(relpath).splitlines(), f"{relpath}: expected {exact!r}")


def main() -> None:
    for endpoint in ("Boat", "Remote"):
        ini = endpoint + "/platformio.ini"
        guard = endpoint + "/tools/pio_deny_upload.py"
        assert_line(ini, "upload_protocol = custom")
        assert_line(ini, "upload_command = python tools/pio_deny_upload.py")
        require("sys.exit(2)" in text(guard), f"{guard}: upload-denial exit removed")

    radio_flags = {
        "Boat/source/boat_radio_hw_config.h": [
            "#define BOAT_NRF_PIN_AUDIT_SIGNED 0",
            "#define BOAT_E22_UART_PIN_AUDIT_SIGNED 0",
            "#define BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED 0",
            "#define BOAT_E22_RF_AIRTIME_AUDIT_SIGNED 0",
        ],
        "Remote/source/radio_hw_config.h": [
            "#define REMOTE_NRF_PIN_AUDIT_SIGNED 0",
            "#define REMOTE_E22_BRIDGE_AUDIT_SIGNED 0",
            "#define REMOTE_E22_RF_INTEROPERABILITY_VERIFIED 0",
            "#define REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED 0",
            "#define REMOTE_NRF_CE_PIN   6",
            "#define REMOTE_SD_DUMMY_CSN_PIN 6",
            "#define REMOTE_NRF_CSN_PIN  15",
        ],
        "Boat/source/boat_r30h9_marker_config.h": [
            "#define BOAT_R30H9_MARKER_GPIO (-1)",
            "#define BOAT_R30H9_GPIO_OUTPUT_SIGNED 0",
        ],
        "Remote/source/remote_r30h9_marker_config.h": [
            "#define REMOTE_R30H9_MARKER_GPIO (-1)",
            "#define REMOTE_R30H9_GPIO_OUTPUT_SIGNED 0",
        ],
    }
    for path, lines in radio_flags.items():
        for line in lines:
            assert_line(path, line)

    path = ROOT / "Boat/docs/BOAT_R30H_10_PHYSICAL_GATES.csv"
    with path.open(encoding="utf-8", newline="") as f:
        rows = list(csv.DictReader(f))
    require(len(rows) == 18, "expected exactly 18 hardware gates")
    observed_ids = {row["Gate_ID"] for row in rows}
    require(observed_ids == {f"H10-{n:02d}" for n in range(1, 19)}, "missing/duplicate gate IDs")
    blocked = {"H10-03", "H10-04", "H10-13", "H10-14", "H10-15", "H10-16"}
    for row in rows:
        gate = row["Gate_ID"]
        status = row["Status"]
        require(status in ("NOT_RUN", "BLOCKED"), f"{gate}: unsupported physical completion {status!r}")
        if gate in blocked:
            require(status == "BLOCKED", f"{gate}: known hard blocker must remain BLOCKED")
        for forbidden in ("Measured_value", "Raw_capture_or_document", "Reviewer", "Review_date"):
            require(not (row.get(forbidden) or "").strip(), f"{gate}: unreviewed evidence supplied")

    print("R30H-10 PASS: all 18 physical gates unsigned, physical RF/STOP denied, upload denied")
    print("NOTE: this PASS is a *source lock check*, NOT a physical test result.")


if __name__ == "__main__":
    main()
