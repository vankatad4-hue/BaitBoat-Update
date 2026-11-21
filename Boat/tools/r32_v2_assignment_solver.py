#!/usr/bin/env python3
"""R32 screenless V2 pin solver. Feasibility only; never authorizes wiring."""
import itertools
import unittest

# Conservative provisional set from R29, minus GPIO21: backlight hazard.
CANDIDATES = frozenset((6, 17, 37, 40, 41, 42, 43, 44))
TFT_SHARED = frozenset((40, 41, 42))
UART_BOOT = frozenset((43, 44))
NATIVE_RADIO = ("sck", "mosi", "miso", "ce", "csn")
I2C = ("sda", "scl")
PWM = ("pwm_left", "pwm_right")
DIRECT_MOTOR = PWM + ("dir_left", "dir_right", "permit", "watchdog")
def budget(*, bridge=False, offload_direction=False, offload_permit_watchdog=False):
    radio = ("ce",) if bridge else NATIVE_RADIO
    motor = PWM + (() if offload_direction else ("dir_left", "dir_right"))
    motor += () if offload_permit_watchdog else ("permit", "watchdog")
    return len(radio) + len(I2C) + len(motor)
def assess(*, bridge=False, offload_direction=False, offload_permit_watchdog=False):
    required=budget(bridge=bridge,offload_direction=offload_direction,
                    offload_permit_watchdog=offload_permit_watchdog)
    return {"native_required":required,"candidate_count":len(CANDIDATES),
            "shortfall":max(0,required-len(CANDIDATES)),
            "electrically_qualified":False,
            "reasons":("TFT shared nets", "UART0 boot states",
                       "V2 header mapping", "PWM/expander safety")}
class Tests(unittest.TestCase):
    def test_current_firmware_direct_count(self):
        self.assertEqual(budget(),13)
    def test_offload_direction_only(self):
        self.assertEqual(budget(offload_direction=True),11)
    def test_offload_direction_and_permit_watchdog(self):
        self.assertEqual(budget(offload_direction=True,offload_permit_watchdog=True),9)
    def test_screenless_eight_candidates_cannot_fit_nine(self):
        self.assertEqual(assess(offload_direction=True,offload_permit_watchdog=True)["shortfall"],1)
    def test_bridge_with_direct_motor(self):
        self.assertEqual(budget(bridge=True),9)
    def test_bridge_plus_direction_offload(self):
        self.assertEqual(budget(bridge=True,offload_direction=True),7)
    def test_no_automatic_approval(self):
        self.assertFalse(assess(bridge=True,offload_direction=True)["electrically_qualified"])
    def test_backlight_excluded(self):
        self.assertNotIn(21,CANDIDATES)
if __name__=="__main__":
    unittest.main()
