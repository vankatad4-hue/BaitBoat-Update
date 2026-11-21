#!/usr/bin/env python3
"""Manufacturing-net contract only. The Cytron connector orientation is unverified."""
import pathlib
import re

CARRIER = {1: "PWM_L", 2: "DIR_L", 3: "PWM_R", 4: "DIR_R", 5: "GND"}
VENDOR_TERMINAL_LABELS = frozenset(("DIR1", "PWM1", "DIR2", "PWM2", "GND"))
# Actual Cytron physical pin numbering is intentionally NOT asserted.
CROSSOVER = {1: "PWM1", 2: "DIR1", 3: "PWM2", 4: "DIR2", 5: "GND"}

def valid_harness(mapping):
    if set(mapping) != set(CARRIER) or set(mapping.values()) != VENDOR_TERMINAL_LABELS:
        return False
    return all(mapping[n] == target for n, target in CROSSOVER.items())

def carrier_schematic_labels(path):
    """Read actual R7 MDD20A harness connector labels, ordered by KiCad pin y."""
    data = pathlib.Path(path).read_text(encoding="utf8")
    entries = [(float(y), net) for net, y in re.findall(
        r'\(label "([^"]+)" \(at 133\.35 ([0-9.]+) 0\)', data)]
    return tuple(net for _, net in sorted(entries))

if __name__ == "__main__":
    print("MDD20A carrier crossover valid:", valid_harness(CROSSOVER))
    print("Physical vendor socket orientation: NOT VERIFIED; no straight-through mating")
