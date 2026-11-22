"""R75 static hardware-plan contract only; physical tests remain NOT_RUN."""
import csv
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
PLAN = ROOT / "Boat/docs/BOAT_R75_HARDWARE_ACCEPTANCE.csv"
REGISTER = ROOT / "docs/PROJECT_PURCHASE_REGISTER_R20.csv"
MOUNT = ROOT / "Boat/docs/BOAT_R75_TSR_SOCKET_HARNESS_AND_FUSE_PLAN.md"
AUDIT = ROOT / "Boat/docs/BOAT_R75_HARDWARE_CONFIRMATION_AUDIT.md"

class R75HardwarePlanContract(unittest.TestCase):
    def test_all_physical_gates_remain_unrun(self):
        with PLAN.open(encoding="utf-8", newline="") as fh:
            rows = list(csv.DictReader(fh))
        self.assertEqual(len(rows), 11)
        self.assertTrue(all(r["Status"] == "NOT_RUN" for r in rows))
        self.assertEqual({r["Test_ID"] for r in rows}, {
            "R75-M01", "R75-M02", "R75-E01", "R75-F01", "R75-P01",
            "R75-P02", "R75-P03", "R75-P04", "R75-P05", "R75-P06", "R75-S01"
        })

    def test_socket_and_connector_are_candidates_not_locked_parts(self):
        with REGISTER.open(encoding="utf-8", newline="") as fh:
            rows = list(csv.DictReader(fh))
        items = {r["Item"]: r for r in rows if r["Side"] == "Boat"}
        self.assertEqual(items["TSR SIP3 primary socket candidate"]["Selected_or_candidate"],
                         "R75A_BETTER_DIMENSIONED_CANDIDATE_SAMPLE_FIT_REQUIRED")
        self.assertEqual(items["TSR SIP3 alternate socket"]["Selected_or_candidate"],
                         "R75A_SECONDARY_SOCKET_CANDIDATE_NO_SQUARE_PIN_SPEC")
        self.assertEqual(items["TSR pluggable harness cable side"]["Exact_part"],
                         "Phoenix Contact MSTB 2.5/3-ST-5.08 order 1757022")
        self.assertEqual(items["TSR pluggable harness header side"]["Exact_part"],
                         "Phoenix Contact MSTBA 2.5/3-G-5.08-RN order 1926028")
        self.assertIn("NOT_PURCHASED", items["TSR 2-2450 recommended input fuse"]["Selected_or_candidate"])
        self.assertIn("CANDIDATE", items["TSR 5x20mm fuse holder"]["Selected_or_candidate"])

    def test_main_distribution_constraints_are_documented(self):
        mount = MOUNT.read_text(encoding="utf-8").lower()
        for phrase in ("no custom electrical pcb", "never parallel", "usb-a", "usb backfeed",
                       "ekm014 vl only", "vh", "| 1 | vin", "| 2 | gnd", "| 3 | +vout"):
            self.assertIn(phrase, mount)
        audit = AUDIT.read_text(encoding="utf-8").lower()
        for phrase in ("independent physical safety", "current-zero", "mdd20a", "ekm014",
                       "m100 pro", "gy-91", "mcp23017", "not purchased"):
            self.assertIn(phrase, audit)
        self.assertEqual((ROOT / "Boat/releases/VERSION").read_text().strip(), "014")
        self.assertEqual((ROOT / "Remote/releases/VERSION").read_text().strip(), "028")

if __name__ == "__main__":
    unittest.main()
