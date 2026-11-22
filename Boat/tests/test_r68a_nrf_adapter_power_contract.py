"""R68A STATIC contract only: Ardboard nRF socket input and E22 separation.

No powered board or real radio-current tests are run by this test file.
"""
from pathlib import Path
import csv
import unittest

ROOT = Path(__file__).resolve().parents[2]

def csv_rows(path):
    with (ROOT / path).open(encoding="utf-8", newline="") as source:
        return list(csv.DictReader(source))

class R68AArdboardAdapterContract(unittest.TestCase):
    def test_one_unapproved_adapter_at_bulgarian_supplier(self):
        items = csv_rows("docs/PROJECT_PURCHASE_REGISTER_R20.csv")
        adapter = [i for i in items if i["Side"] == "Boat"
                   and "Ardboard 24L01 board article 1127" in i["Exact_part"]]
        self.assertEqual(len(adapter), 1)
        self.assertEqual(adapter[0]["Qty"], "0")
        self.assertEqual(adapter[0]["Selected_or_candidate"],
                         "R23_OPTIONAL_SUPERSEDED_POWER_PATH")
        self.assertEqual(adapter[0]["Shop_URL"],
                         "https://www.ardboard.com/nrf24l01-socket-adapter")
        self.assertIn("optional socket", adapter[0]["Purchase_gate"].lower())

    def test_historical_r68_bom_remains_distinct_from_r23_approved_design(self):
        rails = {r["Rail"]: r for r in csv_rows("Boat/docs/BOAT_R68_POWER_RAIL_BOM.csv")}
        self.assertEqual(len(rails), 4)
        self.assertEqual(set(rails), {
            "BOAT_5V_MCU", "BOAT_5V_GATE", "BOAT_3V3_E22",
            "BOAT_3V3_SENSORS"
        })
        self.assertEqual(rails["BOAT_3V3_E22"]["Exact_MPN"], "D24V10F3")
        self.assertIn("E22", rails["BOAT_3V3_E22"]["Loads"])
        self.assertNotIn("nRF", rails["BOAT_3V3_E22"]["Loads"])
        self.assertIn("Ardboard 1127", rails["BOAT_3V3_E22"]["Issue_Before_Purchase"])
        self.assertEqual(rails["BOAT_5V_MCU"]["Exact_MPN"], "DCDC6-16-TO5")
        active = {r["Item"]: r for r in csv_rows("docs/PROJECT_PURCHASE_REGISTER_R20.csv")
                  if r["Side"] == "Boat"}
        self.assertEqual(active["nRF24 primary 3V3 power unit 1"]["Exact_part"],
                         "Pololu D24V10F3 #2830")
        self.assertIn("nRF24 PA/LNA", active["nRF24 primary 3V3 power unit 1"]["Purchase_gate"])
        self.assertIn("E22 VCC supplied", active["nRF24 primary 3V3 power unit 1"]["Purchase_gate"])
        self.assertIn("ADS1115", active["Sensor and I2C expansion 3V3 power"]["Purchase_gate"])
        self.assertEqual(active["nRF24 local socket power adapter"]["Qty"], "0")

    def test_adapter_has_no_3S_direct_connection_or_certified_output_claim(self):
        review = (ROOT / "Boat/docs/BOAT_R68A_ARDBOARD_NRF_SOCKET_POWER_REVIEW.md").read_text(encoding="utf-8")
        for phrase in ("NOT electrical acceptance", "VCC/GND input",
                       "does not state", "5 V", "Do not feed 3S pack voltage directly",
                       "0.196 W", "0.425 W", "0.850 W", "does NOT eliminate",
                       "E22-only", "no firmware pin authorization"):
            self.assertIn(phrase.lower(), review.lower())
        self.assertNotIn("APPROVED FOR ACTUATOR", review)

    def test_hardware_acceptance_still_all_not_run(self):
        rows = csv_rows("Boat/docs/BOAT_R68A_NRF_ADAPTER_TEST_PLAN.csv")
        self.assertEqual(len(rows), 9)
        self.assertTrue(all(r["Status"] == "NOT_RUN" for r in rows))

    def test_unchanged_non_actuating_firmware(self):
        conf = (ROOT / "Boat/source/boat_radio_hw_config.h").read_text()
        self.assertIn("#define BOAT_NRF_PIN_AUDIT_SIGNED 0", conf)
        self.assertIn("#define BOAT_E22_UART_PIN_AUDIT_SIGNED 0", conf)
        sketch = (ROOT / "Boat/source/firmware/Flytec_Boat.ino").read_text()
        self.assertNotIn("application.attach(", sketch)
        self.assertEqual((ROOT / "Boat/releases/VERSION").read_text().strip(), "014")
        self.assertEqual((ROOT / "Remote/releases/VERSION").read_text().strip(), "028")

if __name__ == "__main__":
    unittest.main()
