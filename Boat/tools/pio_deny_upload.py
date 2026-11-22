#!/usr/bin/env python3
"""Fail-closed PlatformIO upload guard for uncommissioned Flytec firmware."""
import sys
sys.stderr.write(
    "FLYTEC: PlatformIO build ONLY. Flash/upload is disabled until the "
    "actual ESP32-S3 board revision, GPIO and RF pins, power rails, "
    "physical STOP and actuator disconnect have signed acceptance. "
    "Use the approved hardware bring-up plan; no release is authorized.\n"
)
sys.exit(2)
