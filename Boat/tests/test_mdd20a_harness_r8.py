import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import mdd20a_harness_r8 as h

class CytronHarnessWiring(unittest.TestCase):
    def test_crossmap_is_1_to_1(self):
        self.assertTrue(h.valid_harness(h.CROSSOVER))
        self.assertEqual(len(h.CROSSOVER), 5)
        self.assertNotEqual(tuple(h.CARRIER.values()),
                            ("DIR1", "PWM1", "DIR2", "PWM2", "GND"))

    def test_swapped_motor_controls_rejected(self):
        wrong = dict(h.CROSSOVER)
        wrong[1], wrong[2] = wrong[2], wrong[1]
        self.assertFalse(h.valid_harness(wrong))
        wrong = dict(h.CROSSOVER)
        wrong[3] = "PWM1"
        self.assertFalse(h.valid_harness(wrong))
        wrong = dict(h.CROSSOVER)
        wrong[5] = "ACTUATOR_BUS"
        self.assertFalse(h.valid_harness(wrong))

    def test_native_kicad_logical_carrier_pins(self):
        actual = h.carrier_schematic_labels(
            ROOT / "hardware" / "Direct_MDD20A_R7" /
            "Boat_MDD20A_Control_R7.kicad_sch")
        self.assertEqual(actual, tuple(h.CARRIER.values()))

if __name__ == "__main__":
    unittest.main()
