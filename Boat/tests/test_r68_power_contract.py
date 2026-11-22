"""R68 static documentation/commissioning guard only; no physical regulator test."""
import csv
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
BOM = ROOT / "Boat/docs/BOAT_R68_POWER_RAIL_BOM.csv"
TESTS = ROOT / "Boat/docs/BOAT_R68_POWER_ACCEPTANCE.csv"

class R68PowerContract(unittest.TestCase):
    def test_four_independent_rail_roles(self):
        with BOM.open(encoding="utf-8", newline="") as fh:
            rows = list(csv.DictReader(fh))
        self.assertEqual(len(rows), 4)
        expected = {
            "BOAT_5V_MCU": ("Olimex", "DCDC6-16-TO5", "5.0"),
            "BOAT_5V_GATE": ("Pololu", "S13V20F5", "5.0"),
            "BOAT_3V3_RF": ("Pololu", "D24V10F3", "3.3"),
            "BOAT_3V3_SENSORS": ("RECOM", "R-78E3.3-0.5", "3.3"),
        }
        self.assertEqual({r["Rail"] for r in rows}, set(expected))
        self.assertEqual({r["Exact_MPN"] for r in rows}, {x[1] for x in expected.values()})
        for r in rows:
            with self.subTest(rail=r["Rail"]):
                self.assertEqual(
                    (r["Manufacturer"], r["Exact_MPN"], r["Output_V"]), expected[r["Rail"]]
                )
                self.assertEqual(r["Qty"], "1")
                self.assertEqual(r["Supply_Audit_Status"], "R68_PROPOSED_NOT_OWNER_CONFIRMED")
                self.assertLessEqual(float(r["Vin_Min_V"]), 9.0)
                self.assertGreaterEqual(float(r["Vin_Max_V"]), 12.6)
        self.assertIn("EKM014 VL only", next(r["Loads"] for r in rows if r["Rail"] == "BOAT_5V_GATE"))
        self.assertNotIn("ESP32", next(r["Loads"] for r in rows if r["Rail"] == "BOAT_3V3_SENSORS"))

    def test_safety_rails_are_distinct_and_not_commissioned(self):
        text = (ROOT / "Boat/docs/BOAT_R68_3V3_5V_REGULATOR_SELECTION.md").read_text(encoding="utf-8")
        for item in ("No cross-feed", "do NOT use an external 3.3 V regulator",
                     "three mutually exclusive supply options", "one 5 V injection path",
                     "MDD20A", "EKM014 VH", "not owner-confirmed",
                     "No custom PCB", "no live GPIO"):
            self.assertIn(item.lower(), text.lower())
        radio = (ROOT / "Boat/source/boat_radio_hw_config.h").read_text(encoding="utf-8")
        for item in ("#define BOAT_NRF_PIN_AUDIT_SIGNED 0",
                     "#define BOAT_E22_UART_PIN_AUDIT_SIGNED 0"):
            self.assertIn(item, radio)
        firmware = (ROOT / "Boat/source/firmware/Flytec_Boat.ino").read_text(encoding="utf-8")
        self.assertNotIn("application.attach(", firmware)
        self.assertEqual((ROOT / "Boat/releases/VERSION").read_text().strip(), "014")
        self.assertEqual((ROOT / "Remote/releases/VERSION").read_text().strip(), "028")

    def test_twelve_hardware_checks_not_run(self):
        with TESTS.open(encoding="utf-8", newline="") as fh:
            tests = list(csv.DictReader(fh))
        self.assertEqual(len(tests), 12)
        self.assertTrue(all(t["State"] == "NOT_RUN" for t in tests))

    def test_purchase_register_proposal_not_owner_selected(self):
        register = ROOT / "docs/PROJECT_PURCHASE_REGISTER_R20.csv"
        with register.open(encoding="utf-8", newline="") as fh:
            rows = list(csv.DictReader(fh))
        statuses = {
            p["Exact_part"].split(" ")[0]: p["Selected_or_candidate"]
            for p in rows
            if p["Side"] == "Boat"
        }
        for part in ("DCDC6-16-TO5", "S13V20F5", "D24V10F3", "R-78E3.3-0.5"):
            self.assertEqual(statuses.get(part), "R68_PROPOSED_NOT_OWNER_CONFIRMED", part)

if __name__ == "__main__":
    unittest.main()
