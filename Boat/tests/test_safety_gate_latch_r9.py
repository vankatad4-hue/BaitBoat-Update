import dataclasses
import itertools
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from safety_gate_latch_r9 import (InterlockSample, PhysicalFaultLatchModel,
                                  OneHotPulseLimiterModel, safe_example)

FAULT_NAMES = ("stop_closed", "watchdog_ok", "uvlo_ok",
               "thermal_ok", "current_ok", "rails_good",
               "output_feedback_valid")


def arm_after_release(latch):
    good = safe_example()
    assert not latch.step(good, physical_arm_pressed=False, mcu_permit=True)
    return latch.step(good, physical_arm_pressed=True, mcu_permit=True)


class PhysicalLatchRegression(unittest.TestCase):
    def test_all_128_fault_combinations(self):
        for combo in itertools.product((False, True), repeat=len(FAULT_NAMES)):
            fb = dataclasses.replace(safe_example(), **dict(zip(FAULT_NAMES, combo)))
            l = PhysicalFaultLatchModel()
            l.step(fb, physical_arm_pressed=False, mcu_permit=True)
            armed = l.step(fb, physical_arm_pressed=True, mcu_permit=True)
            self.assertEqual(armed, all(combo), (FAULT_NAMES, combo))
            if not all(combo):
                self.assertFalse(l.step(safe_example(), physical_arm_pressed=True,
                                        mcu_permit=True))  # stuck button
                self.assertFalse(l.step(safe_example(), physical_arm_pressed=False,
                                        mcu_permit=True))
                self.assertTrue(l.step(safe_example(), physical_arm_pressed=True,
                                       mcu_permit=True))

    def test_startup_held_button_denied_until_released(self):
        l = PhysicalFaultLatchModel()
        self.assertFalse(l.step(safe_example(), physical_arm_pressed=True,
                                mcu_permit=True))
        self.assertFalse(l.step(safe_example(), physical_arm_pressed=True,
                                mcu_permit=True))
        self.assertFalse(l.step(safe_example(), physical_arm_pressed=False,
                                mcu_permit=True))
        self.assertTrue(l.step(safe_example(), physical_arm_pressed=True,
                               mcu_permit=True))

    def test_arm_requires_stationary_and_idle(self):
        for bad in ({"motors_idle": False}, {"solenoids_idle": False},
                    {"output_feedback_valid": False}):
            l = PhysicalFaultLatchModel()
            s = dataclasses.replace(safe_example(), **bad)
            self.assertFalse(l.step(s, physical_arm_pressed=False, mcu_permit=True))
            self.assertFalse(l.step(s, physical_arm_pressed=True, mcu_permit=True))

    def test_permit_cannot_rearm_fault(self):
        l = PhysicalFaultLatchModel()
        self.assertTrue(arm_after_release(l))
        self.assertFalse(l.step(safe_example(), physical_arm_pressed=False,
                                mcu_permit=False))
        self.assertTrue(l.step(safe_example(), physical_arm_pressed=False,
                               mcu_permit=True))  # permit does not reset latch
        bad = dataclasses.replace(safe_example(), watchdog_ok=False)
        self.assertFalse(l.step(bad, physical_arm_pressed=True, mcu_permit=True))
        self.assertFalse(l.step(safe_example(), physical_arm_pressed=True,
                                mcu_permit=True))
        self.assertFalse(l.step(safe_example(), physical_arm_pressed=False,
                                mcu_permit=True))
        self.assertTrue(l.step(safe_example(), physical_arm_pressed=True,
                               mcu_permit=True))


class IndependentPulseLimiterRegression(unittest.TestCase):
    def test_four_single_pulses_and_time_limit(self):
        p = OneHotPulseLimiterModel(80)  # synthetic not a hardware value
        for ch in range(4):
            mask = 1 << ch
            self.assertEqual(p.step(ch * 100, mask, gate_power_good=True,
                                    motors_idle=True, coils_zero_current=True), mask)
            self.assertEqual(p.step(ch * 100 + 79, mask, gate_power_good=True,
                                    motors_idle=True, coils_zero_current=False), mask)
            self.assertEqual(p.step(ch * 100 + 80, mask, gate_power_good=True,
                                    motors_idle=True, coils_zero_current=False), 0)
            self.assertEqual(p.step(ch * 100 + 81, mask, gate_power_good=True,
                                    motors_idle=True, coils_zero_current=True), 0)
            self.assertEqual(p.step(ch * 100 + 82, 0, gate_power_good=True,
                                    motors_idle=True, coils_zero_current=True), 0)

    def test_overlap_and_stuck_power_fault_latches(self):
        for mask in (3, 5, 7, 15):
            p = OneHotPulseLimiterModel(30)
            self.assertEqual(p.step(1, mask, gate_power_good=True,
                                    motors_idle=True, coils_zero_current=True), 0)
            self.assertTrue(p.fault)
            self.assertEqual(p.step(2, 1, gate_power_good=True,
                                    motors_idle=True, coils_zero_current=True), 0)
        for fail in ({"gate_power_good": False}, {"motors_idle": False}):
            p = OneHotPulseLimiterModel(30)
            args = dict(gate_power_good=True, motors_idle=True,
                        coils_zero_current=True)
            args.update(fail)
            self.assertEqual(p.step(1, 1, **args), 0)
            self.assertTrue(p.fault)

    def test_changed_channel_aborts_and_requires_zero(self):
        p = OneHotPulseLimiterModel(80)
        good = dict(gate_power_good=True, motors_idle=True, coils_zero_current=True)
        self.assertEqual(p.step(0, 1, **good), 1)
        self.assertEqual(p.step(1, 2, **good), 0)
        self.assertEqual(p.step(2, 2, **good), 0)
        self.assertEqual(p.step(3, 0, **good), 0)
        self.assertEqual(p.step(4, 2, **good), 2)


if __name__ == "__main__":
    unittest.main()
