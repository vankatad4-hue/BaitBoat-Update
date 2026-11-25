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

    # Guard the PlatformIO source adapter contract against duplicate setup/loop.
    boat_pio = text("Boat/platformio.ini")
    remote_pio = text("Remote/platformio.ini")
    require("-<firmware/**>" in boat_pio, "Boat nested sketch exclusion missing")
    require("+<*.ino>" in remote_pio, "Remote root sketch exclusion regression")
    require('#include "firmware/Flytec_Boat.ino"' in text("Boat/source/platformio_entry.cpp"),
            "Boat guarded sketch adapter missing")
    require("FLYTEC_PLATFORMIO_BUILD" in text("Boat/source/platformio_entry.cpp"),
            "Boat adapter must remain guarded from Arduino CLI")
    require(not (ROOT / "Remote/source/platformio_entry.cpp").exists(),
            "Remote duplicate setup()/loop() adapter reappeared")
    radio_flags = {
        "Boat/source/boat_radio_hw_config.h": [
            "#define BOAT_NRF_PIN_AUDIT_SIGNED 0",
            "#define BOAT_E22_UART_PIN_AUDIT_SIGNED 0",
            "#define BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED 0",
            "#define BOAT_E22_RF_AIRTIME_AUDIT_SIGNED 0",
        ],
        "Remote/source/radio_hw_config.h": [
            "#define REMOTE_NRF_PIN_AUDIT_SIGNED 0",
            "#define REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED 0",
            "#define REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED 0",
            "#define REMOTE_NRF_BRIDGE_I2C_ADDRESS 0x28",
            "#define REMOTE_NRF_CE_EXPANDER_ADDRESS 0x21",
            "#define REMOTE_NRF_CE_EXPANDER_GPA 0",
            "#define REMOTE_E22_BRIDGE_AUDIT_SIGNED 0",
            "#define REMOTE_E22_RF_INTEROPERABILITY_VERIFIED 0",
            "#define REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED 0",
            "#define REMOTE_NRF_CE_PIN   (-1)",
            "#define REMOTE_SD_DUMMY_CSN_PIN 6",
            "#define REMOTE_NRF_CSN_PIN  (-1)",
            "#define REMOTE_NRF_MOSI_PIN (-1)",
            "#define REMOTE_NRF_SCK_PIN  (-1)",
            "#define REMOTE_NRF_MISO_PIN (-1)",
            "#define REMOTE_TF_SPI_MOSI_PIN 11",
            "#define REMOTE_TF_SPI_SCK_PIN 12",
            "#define REMOTE_TF_SPI_MISO_PIN 13",
            "#define REMOTE_R30H10_REJECTED_CE_GPIO 6",
            "#define REMOTE_R30H10_REJECTED_CSN_GPIO 15",
        ],
        "Boat/source/boat_diagnostic_marker_config.h": [
            "#define BOAT_R30H9_MARKER_GPIO (-1)",
            "#define BOAT_R30H9_GPIO_OUTPUT_SIGNED 0",
        ],
        "Remote/source/remote_diagnostic_marker_config.h": [
            "#define REMOTE_R30H9_MARKER_GPIO (-1)",
            "#define REMOTE_R30H9_GPIO_OUTPUT_SIGNED 0",
        ],
    }
    for path, lines in radio_flags.items():
        for line in lines:
            assert_line(path, line)

    from verify_r30h10_gpio_map import check_gpio_map
    require(not check_gpio_map(ROOT), "R30H-10 GPIO map source check failed")
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

    # Separate R30H-12 test ledger: never turn code-only CI into simulated
    # measurements or silent RF signoff. H10's 18-row register is unchanged.
    extra = ROOT / "Remote/docs/REMOTE_R30H_12_PHYSICAL_MEASUREMENTS.csv"
    with extra.open(encoding="utf-8", newline="") as f:
        bridge_rows = list(csv.DictReader(f))
    require(len(bridge_rows) == 12, "R30H-12 requires exactly 12 physical probes")
    require({r["Gate_ID"] for r in bridge_rows} ==
            {f"R12-{n:02d}" for n in range(1, 13)}, "R30H-12 missing or duplicate probe IDs")
    for record in bridge_rows:
        require(record["Status"] in ("NOT_RUN", "BLOCKED"),
                f'{record["Gate_ID"]}: synthetic R30H-12 completion')
        for evidence_field in ("Measured_value", "Raw_capture_or_document", "Reviewer", "Review_date"):
            require(not (record.get(evidence_field) or "").strip(),
                    f'{record["Gate_ID"]}: unreviewed physical data')
    print("R30H-10 PASS: 18 original gates and 12 R30H-12 probes unsigned; upload denied")
    print("NOTE: this PASS is a *source lock check*, NOT a physical test result.")


if __name__ == "__main__":
    main()
