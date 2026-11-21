#!/usr/bin/env python3
"""R8 only: offline Heltec V1.2 GPIO *candidates*, never firmware pin assignments.

The evidence is the Heltec Wireless Tracker V1.1 published map/schematic, NOT
a signed V1.2 board. Keep all physical GPIO registers set to -1 until a board
review, motor safety wiring and live SPI/boot tests are completed.
"""
from typing import Mapping

# Native logical interfaces. No external Boat E22 UART.
SIGNALS = frozenset((
    "independent_gate_permit", "external_watchdog_wdi",
    "nrf_ce", "nrf_csn", "i2c_sda", "i2c_scl",
    "mdd_pwm_left", "mdd_pwm_right", "mdd_dir_left", "mdd_dir_right",
))
# Published V1.1 candidates; NOT verified electrically on V1.2.
CANDIDATES = frozenset((4, 5, 6, 7, 15, 16, 17, 18, 37, 43, 44))
# SX1262 8..14; board GNSS 33..36; TFT 21/38..42;
# USB 19/20; battery/enable 1/2/3; boot straps 0,45,46.
RESERVED = frozenset((0, 1, 2, 3, 8, 9, 10, 11, 12, 13, 14,
                      19, 20, 21, 26, 33, 34, 35, 36,
                      38, 39, 40, 41, 42, 45, 46, 47, 48))
CONTESTED = frozenset((15, 16, 18, 37, 43, 44))
# Both keep independent safety GPIOs away from contested pins.
BASE = {
    "independent_gate_permit": 4,
    "external_watchdog_wdi": 17,
    "nrf_csn": 5,
    "i2c_sda": 6,
    "i2c_scl": 7,
    "mdd_pwm_left": 15,
    "mdd_pwm_right": 16,
    "mdd_dir_left": 43,
    "mdd_dir_right": 44,
}
PROPOSALS = {
    "A_LED18": dict(BASE, nrf_ce=18),  # GPIO18 LED loading to prove
    "B_GPIO37": dict(BASE, nrf_ce=37),  # GPIO37 flash/DQS to prove
}


def validate_proposal(mapping: Mapping[str, int]) -> bool:
    """Only structural feasibility; NEVER a physical safety endorsement."""
    if set(mapping) != SIGNALS:
        return False
    pins = list(mapping.values())
    if any(type(p) is not int for p in pins):
        return False
    if len(set(pins)) != len(SIGNALS):
        return False
    if not set(pins).issubset(CANDIDATES) or set(pins) & RESERVED:
        return False
    return mapping["independent_gate_permit"] not in CONTESTED and (
        mapping["external_watchdog_wdi"] not in CONTESTED
    )


def may_energize_actuators(_: Mapping[str, int]) -> bool:
    """No present V1.2 board audit, electrical latch or physical tests."""
    return False


if __name__ == "__main__":
    import json
    print(json.dumps({
        "board": "Heltec Wireless Tracker V1.2 (unverified)",
        "status": "REJECTED for physical use; V1.1 reference-only",
        "signals": len(SIGNALS),
        "proposals": {name: {"structurally_valid": validate_proposal(pins),
                             "hardware_approved": may_energize_actuators(pins),
                             "gpio": pins} for name, pins in PROPOSALS.items()},
        "shared_spi": "Internal SX1262 pins 9/10/11; nRF external SPI access unverified",
    }, indent=2))
