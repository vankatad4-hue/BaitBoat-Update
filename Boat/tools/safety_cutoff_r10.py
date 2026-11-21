#!/usr/bin/env python3
"""Host-only R10 logical fault tree. Not an electronic circuit simulation."""
from dataclasses import dataclass
from typing import Tuple

SAFE_NAMES = (
    "STOP_OK_NC_SECOND_POLE", "WDO_N", "UVLO_OK", "CURRENT_OK",
    "TEMP_OK", "RAIL_OK", "MOTOR_IDLE", "COIL_IDLE",
    "OUTPUT_FB_OK", "POR_OK"
)

@dataclass(frozen=True)
class SafetyInputs:
    safe: Tuple[bool, ...]
    physical_stop_nc_closed: bool
    mcu_permit: bool
    physical_arm_pressed: bool
    def valid(self):
        return len(self.safe) == len(SAFE_NAMES) and all(type(x) is bool for x in self.safe)

class CutoffLogicModel:
    def __init__(self):
        self.latch=False
        self.last_arm=False
        self.qualified_arm_released=False
        self.shutdowns=0
    def step(self, x: SafetyInputs) -> bool:
        if not x.valid():
            self.latch=False
            self.qualified_arm_released=False
            self.shutdowns+=1
            return False
        # Actual R10 nets form ten-input AND -> async CLEAR of HC74.
        safe=all(x.safe)
        # Physical NC stop contact also breaks Q2's return, independently
        # of the separate STOP_OK logic input.
        if not safe:
            self.latch=False
            self.qualified_arm_released=False
            self.shutdowns+=1
        else:
            if not x.physical_arm_pressed and x.physical_stop_nc_closed:
                self.qualified_arm_released=True
            rising=x.physical_arm_pressed and not self.last_arm
            if rising and self.qualified_arm_released and x.physical_stop_nc_closed:
                self.latch=True
                self.qualified_arm_released=False
        self.last_arm=x.physical_arm_pressed
        return safe and self.latch and x.mcu_permit and x.physical_stop_nc_closed

def safe_frame(arm=False,permit=True,stop=True):
    return SafetyInputs((True,) * len(SAFE_NAMES),stop,permit,arm)
