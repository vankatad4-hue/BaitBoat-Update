#!/usr/bin/env python3
"""R30H-13 repo-wide, READ-ONLY active Boat+Remote PlatformIO source audit.

Every active .h/.cpp/.c/.ino is inspected; legacy historical material remains
outside the compiler roots. This is static analysis, not functional or physical
hardware acceptance. No keys, upload, GPIO, motor or RF operations.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
COMMON_PAIRS = (
    "e22_uart_link.h",
    "mission_control_wire.h",
    "mission_protocol.cpp", "mission_protocol.h", "mission_stick.h",
    "mission_wire.h", "r30h6_utc_marker_policy.h",
    "r30h9_marker_contract.h", "radio_command_codec.h",
    "radio_handshake.h", "radio_key_store.cpp",
    "radio_protocol.cpp", "radio_protocol.h",
    "radio_safety_diagnostic_v1.h",
    "radio_session_platform.h", "radio_session_v2.h",
    "radio_utc_time_v1.h",
)
SOURCE_EXT = (".h", ".cpp", ".c", ".ino")

def audit(root: Path = ROOT, overrides: dict[str, str] | None = None):
    overrides = overrides or {}
    errors = []
    report = {
        "evidence": "SOURCE_ONLY_NOT_HARDWARE_ACCEPTANCE",
        "primary_toolchain": "VS Code + PlatformIO",
        "endpoint_source_files": {},
        "endpoint_compile_roots": {},
        "cross_endpoint_shared_pairs_checked": len(COMMON_PAIRS),
        "unresolved_local_includes_for_manual_review": [],
        "deprecated_remote_direct_nrf_sources": 0,
        "physical_gpio_rf_stop_acceptance": "NOT_RUN/BLOCKED",
    }
    def read(path: str) -> str:
        if path in overrides:
            return overrides[path]
        try:
            return (root / path).read_text(encoding="utf-8")
        except (OSError, UnicodeError) as exc:
            errors.append(f"{path}: unavailable or non-UTF8: {exc}")
            return ""

    config = {}
    existing = {}
    for endpoint in ("Boat", "Remote"):
        pio = read(f"{endpoint}/platformio.ini")
        config[endpoint] = pio
        root_src = root / endpoint / "source"
        files = sorted(x for x in root_src.rglob("*")
                       if x.is_file() and x.suffix in SOURCE_EXT)
        existing[endpoint] = files
        report["endpoint_source_files"][endpoint] = len(files)
        report["endpoint_compile_roots"][endpoint] = len([
            x for x in root_src.iterdir()
            if x.is_file() and x.suffix in (".cpp", ".c", ".ino")
        ])
        if len(files) < 95:
            errors.append(f"{endpoint}: implausibly small active source tree: {len(files)}")
        for file in files:
            rel = file.relative_to(root).as_posix()
            content = read(rel)
            if "\x00" in content or re.search(r"^(?:<{7}|={7}|>{7})", content, re.M):
                errors.append(f"{rel}: merge marker or binary NUL in source")
            for name in re.findall(r'^\s*#\s*include\s*"([^"]+)"', content, re.M):
                if (root_src / name).exists() or (file.parent / name).exists():
                    continue
                # External quote-style imports can be injected by LVGL or
                # Espressif; keep report for review rather than inventing
                # a missing-header compile failure.
                report["unresolved_local_includes_for_manual_review"].append(
                    f"{rel}: {name}"
                )

        for required in ("platform = espressif32 @ 8.0.0",
                         "framework = arduino", "board = esp32-s3-devkitc-1",
                         "upload_protocol = custom",
                         "upload_command = python tools/pio_deny_upload.py",
                         "-D FLYTEC_PLATFORMIO_BUILD=1",
                         "lib_ldf_mode = deep"):
            if required not in pio:
                errors.append(f"{endpoint}/platformio.ini: missing {required!r}")
        if ("pio_deny_upload.py" not in pio or
                "sys.exit(2)" not in read(f"{endpoint}/tools/pio_deny_upload.py")):
            errors.append(f"{endpoint}: denied upload guard missing")
    # Prevent broad, silent migration to a different compiler/sketch model.
    if "-<firmware/**>" not in config["Boat"] or "+<*.ino>" not in config["Remote"]:
        errors.append("Boat guarded C++ / Remote root .ino source filters drift")
    if '#include "firmware/Flytec_Boat.ino"' not in read("Boat/source/platformio_entry.cpp"):
        errors.append("Boat PlatformIO guarded sketch adapter missing")
    if (root / "Remote/source/platformio_entry.cpp").exists():
        errors.append("Remote duplicate setup()/loop() adapter forbidden")
    for path in ("Boat/source/firmware/Flytec_Boat.ino",
                 "Remote/source/Flytec_Remote.ino"):
        content=read(path)
        if (len(re.findall(r"\bvoid\s+setup\s*\(", content)) != 1 or
                len(re.findall(r"\bvoid\s+loop\s*\(", content)) != 1):
            errors.append(f"{path}: must have one setup() and loop()")
    if "nrf24/RF24 @ 1.4.11" not in config["Boat"]:
        errors.append("Boat: active RF24 library pin lost")
    if "nrf24/RF24" in config["Remote"]:
        errors.append("Remote: obsolete RF24 SPI library reintroduced")
    if 'arduino-cli lib install "RF24@1.4.11"' in read(
            ".github/workflows/remote-target-build.yml"):
        errors.append("Remote Arduino reference still downloads dead RF24")
    if ("nrf24/RF24" in read("Boat/platformio.ini") and
            "RF24.h" not in read("Boat/source/nrf24_link.h")):
        errors.append("Boat: direct RF24 firmware unexpectedly removed")
    retired = ["Remote/source/nrf24_link.cpp", "Remote/source/nrf24_link.h"]
    report["deprecated_remote_direct_nrf_sources"] = sum(
        (root / path).exists() for path in retired
    )
    if report["deprecated_remote_direct_nrf_sources"]:
        errors.append("Remote: direct nRF24 driver still active in PlatformIO root")
    if not (root / "Remote/archive/2026-11-23/R30H_13_direct_RF24/nrf24_link.cpp").exists():
        errors.append("Remote: retired RF24 historical copy missing")
    for file in existing["Remote"]:
        rel=file.relative_to(root).as_posix()
        c=read(rel)
        if (re.search(r'#\s*include\s*[<"]RF24.h[>"]',c) or
                re.search(r'\bNRF24Link\b',c)):
            errors.append(f"{rel}: deprecated direct SPI radio dependency")
    for filename in COMMON_PAIRS:
        boat=f"Boat/source/{filename}"
        remote=f"Remote/source/{filename}"
        if (hashlib.sha256(read(boat).encode()).digest()!=
                hashlib.sha256(read(remote).encode()).digest()):
            errors.append(f"shared protocol source drift: {filename}")
    for endpoint in ("Boat", "Remote"):
        if "sys.exit(2)" not in read(f"{endpoint}/tools/pio_deny_upload.py"):
            errors.append(f"{endpoint}: custom upload must fail")
    for source, required in (
        ("Boat/source/nrf24_link.h", "TX_PENDING_TIMEOUT_MS = 120U"),
        # R30H-14: cross-endpoint over-air profile must not drift silently.
        ("Boat/source/boat_radio_hw_config.h", "#define BOAT_NRF_CHANNEL 76"),
        ("Remote/source/radio_hw_config.h", "#define REMOTE_NRF_CHANNEL  76"),
        ("Boat/source/nrf24_link.cpp", "radio_.setDataRate(RF24_250KBPS);"),
        ("Boat/source/nrf24_link.cpp", "radio_.setPALevel(RF24_PA_LOW);"),
        ("Boat/source/nrf24_link.cpp", "radio_.setRetries(5,15);"),
        ("Boat/source/nrf24_link.cpp", "radio_.setCRCLength(RF24_CRC_16);"),
        ("Boat/source/nrf24_link.cpp", "radio_.setPayloadSize(RADIO_FRAME_SIZE);"),
        ("Boat/source/nrf24_link.cpp", "radio_.openReadingPipe(1,rxAddress_);"),
        ("Remote/source/sc18is606_nrf_link.cpp", "!regWrite(REG_SETUP_RETR,0x5f)"),
        ("Remote/source/sc18is606_nrf_link.cpp", "!regWrite(REG_RF_SETUP,0x22)"),
        ("Remote/source/sc18is606_nrf_link.cpp", "!regWrite(REG_EN_AA,0x03)"),
        # R30H-15 STOP may not be lost when the prior RF24 TX owns the slot.
        ("Remote/source/boat_radio.cpp", "g_primaryStopRetry.queue(command,now);"),
        ("Remote/source/boat_radio.cpp", "g_primaryStopRetry.noteAdmitted(now);"),
        ("Remote/source/boat_radio.cpp", "g_primaryStopRetry.noteAdmitted(millis());"),
        ("Remote/source/boat_radio.cpp", "g_primaryStopRetry.reset();\n                }"),
        ("Remote/source/r30h15_primary_stop_retry.h", "MAX_TRANSMISSIONS=8U"),
        ("Remote/source/r30h15_primary_stop_retry.h", "RETRANSMIT_GAP_MS=150U"),
        ("Remote/source/boat_radio.cpp", "g_primaryStopRetry.mayAttempt(sessionReady(),g_nrfReady,"),
        ("Remote/source/boat_radio.cpp", "g_primaryStopRetry.maySendPeriodic(g_nrfReady,g_nrf.txPending())"),
        ("Remote/source/boat_radio.cpp", "g_primaryStopRetry.reset();g_missionId=0;"),
        ("Remote/source/sc18is606_nrf_link.cpp", "!regWrite(REG_RX_PW_P1,(uint8_t)RADIO_FRAME_SIZE)"),
        ("Boat/source/boat_dual_radio_port.cpp", "if(!nrf_.ready())"),
        ("Remote/source/boat_radio.cpp", "Sc18is606NrfLink g_nrf("),
        ("Remote/source/boat_radio.cpp", "REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED &&"),
        ("Remote/source/radio_hw_config.h", "#define REMOTE_NRF_CE_PIN   (-1)"),
        ("Remote/source/radio_hw_config.h", "#define REMOTE_NRF_CSN_PIN  (-1)"),
        ("Remote/source/radio_hw_config.h", "#define REMOTE_NRF_PIN_AUDIT_SIGNED 0"),
        ("Remote/source/radio_hw_config.h", "#define REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED 0"),
        ("Remote/source/radio_hw_config.h", "#define REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED 0"),
        ("Boat/source/boat_radio_hw_config.h", "#define BOAT_NRF_PIN_AUDIT_SIGNED 0"),
    ):
        if required not in read(source):
            errors.append(f"{source}: missing safety/source invariant {required!r}")
    # R30H-15: source order is a safety property. The driver MUST release
    # TX completions and attempt pending STOP BEFORE optional heartbeat slots.
    remote_radio=read("Remote/source/boat_radio.cpp")
    update_start=remote_radio.find("void boatRadio_update()")
    update_end=remote_radio.find("bool boatRadio_utcNow",update_start)
    sequence=(
        "g_nrf.update();",
        "g_primaryStopRetry.mayAttempt(sessionReady(),g_nrfReady,",
        "g_primaryStopRetry.maySendPeriodic(g_nrfReady,g_nrf.txPending())",
        "g_lastPrimaryHeartbeat>=100U",
    )
    positions=[remote_radio.find(word,update_start,update_end) for word in sequence]
    if (update_start<0 or update_end<0 or
            any(pos<0 for pos in positions) or positions!=sorted(positions) or
            len(set(positions))!=len(positions)):
        errors.append("Remote STOP priority broken: poll -> STOP retry -> heartbeat required")
    for endpoint in ("Boat", "Remote"):
        if not (root / f"{endpoint}/releases/VERSION").exists():
            errors.append(f"{endpoint}: release version marker missing")
    report["errors"] = errors
    report["verdict"] = "PASS_SOURCE_ONLY" if not errors else "FAIL_SOURCE_ONLY"
    return errors, report

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--json",type=Path,default=None)
    args=parser.parse_args()
    failures,report=audit()
    if args.json:
        args.json.parent.mkdir(parents=True,exist_ok=True)
        args.json.write_text(json.dumps(report,indent=2,ensure_ascii=False)+"\n",
                             encoding="utf-8")
    print("R30H-13:", report["verdict"])
    print("Active files Boat/Remote:",report["endpoint_source_files"],
          "protocol pairs:",report["cross_endpoint_shared_pairs_checked"])
    print("Unresolved quote-style includes (review only):",
          len(report["unresolved_local_includes_for_manual_review"]))
    for failure in failures:print("ERROR:",failure)
    return bool(failures)

if __name__ == "__main__":
    raise SystemExit(main())
