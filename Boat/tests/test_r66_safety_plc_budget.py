"""R66 host-only capacity contract. No hardware logic or pulse timing is commissioned."""
from pathlib import Path
import csv
import unittest

ROOT = Path(__file__).resolve().parents[2]

def table(name):
    with (ROOT / "Boat/docs" / name).open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))

class R66ControllerCapacity(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.roles = table("BOAT_R66_INTERLOCK_RESOURCE_REQUIREMENTS.csv")
        cls.parts = {r["Candidate"]: r for r in table("BOAT_R66_PLC_CAPACITY_CANDIDATES.csv")}
    def test_required_signals_unique_and_external_cutoff(self):
        self.assertEqual(len(self.roles), 24)
        self.assertEqual(len({r["Signal"] for r in self.roles}), 24)
        inputs = [r for r in self.roles if r["Direction"] == "Input"]
        outputs = [r for r in self.roles if r["Direction"] == "Output"]
        self.assertEqual(len(inputs), 18)
        self.assertEqual(len(outputs), 6)
        self.assertEqual(len([r for r in outputs if r["Candidate_Implementer"] == "4_CHANNEL_PLC"]), 4)
        self.assertEqual({r["Signal"] for r in outputs if r["Candidate_Implementer"] == "EXTERNAL_SAFETY_CHAIN"},
                         {"MOTOR_BRANCH_ISOLATION", "COIL_BRANCH_ISOLATION"})
    def test_pilz_B0_only_fits_with_separate_disconnect(self):
        b0 = self.parts["PNOZ_M_B0"]
        self.assertGreaterEqual(int(b0["Safe_Inputs"]), 18)
        self.assertEqual(int(b0["Safe_Outputs"]), 4)
        self.assertLess(int(b0["Safe_Outputs"]), 6)  # no standalone isolation
        self.assertEqual(b0["Max_Manufacturer_W"], "UNKNOWN")
        self.assertEqual(b0["Design_State"], "RESEARCH_ONLY")
    def test_small_plc_candidates_do_not_fit(self):
        for name in ("PNOZ_M_C0", "G9SP_N10S"):
            with self.subTest(name=name):
                p = self.parts[name]
                self.assertLess(int(p["Safe_Inputs"]), 18)
                self.assertLess(int(p["Safe_Outputs"]), 6)
                self.assertEqual(p["Design_State"], "NOT_FIT_CURRENT_INPUT_BUDGET")
    def test_gate_disallows_unreviewed_purchase(self):
        self.assertTrue(all(p["Design_State"] != "APPROVED" for p in self.parts.values()))
        status = (ROOT/"Boat/docs/BOAT_R66_SAFETY_ARCHITECTURE_REVIEW.md").read_text(encoding="utf-8")
        for word in ("NO_ACTUATOR_ENABLE", "24V", "3S", "E22", "EKM014"):
            self.assertIn(word, status)
        radio = (ROOT/"Boat/source/boat_radio_hw_config.h").read_text(encoding="utf-8")
        self.assertIn("#define BOAT_NRF_PIN_AUDIT_SIGNED 0", radio)
        self.assertIn("#define BOAT_E22_UART_PIN_AUDIT_SIGNED 0", radio)
        firmware = (ROOT/"Boat/source/firmware/Flytec_Boat.ino").read_text(encoding="utf-8")
        self.assertNotIn("application.attach(", firmware)
        self.assertEqual((ROOT/"Boat/releases/VERSION").read_text().strip(), "014")

if __name__ == "__main__":
    unittest.main()
