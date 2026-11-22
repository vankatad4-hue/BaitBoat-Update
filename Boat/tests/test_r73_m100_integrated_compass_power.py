"""R73: host-only consistency checks for integrated M100 PRO QMC5883L.

No physical interface, level-shifter or regulator tests are performed.
"""
import csv
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]

def read_csv(path):
    with (ROOT / path).open(newline="", encoding="utf-8") as fp:
        return list(csv.DictReader(fp))

class R73M100IntegratedCompass(unittest.TestCase):
    def test_compass_not_separately_purchased(self):
        rows = read_csv("docs/PROJECT_PURCHASE_REGISTER_R20.csv")
        gps = [r for r in rows if r["Side"] == "Boat" and r["Item"] == "GNSS"]
        self.assertEqual(len(gps), 1)
        self.assertEqual(gps[0]["Qty"], "1")
        self.assertIn("onboard QMC5883L", gps[0]["Exact_part"])
        self.assertFalse(any(r["Side"] == "Boat"
                             and "QMC5883L" in r["Exact_part"]
                             and r["Item"] != "GNSS" for r in rows))

    def test_5v_gps_and_separate_sensor_power(self):
        rails = {r["Rail"]:r for r in read_csv("Boat/docs/BOAT_R68_POWER_RAIL_BOM.csv")}
        self.assertEqual(rails["BOAT_5V_MCU"]["Output_V"], "5.0")
        self.assertIn("HGLRC M100 PRO", rails["BOAT_5V_MCU"]["Loads"])
        self.assertIn("onboard QMC5883L", rails["BOAT_5V_MCU"]["Loads"])
        sensors = rails["BOAT_3V3_SENSORS"]
        self.assertEqual(sensors["Exact_MPN"], "D24V10F3")
        self.assertEqual(sensors["Output_V"], "3.3")
        self.assertIn("GY-91", sensors["Loads"])
        self.assertIn("MCP23017", sensors["Loads"])
        self.assertNotIn("QMC5883L", sensors["Loads"])
        self.assertEqual(rails["BOAT_3V3_E22"]["Exact_MPN"], "D24V10F3")

    def test_m100_i2c_is_an_included_compass(self):
        rows = read_csv("Boat/docs/BOAT_R67_I2C_AND_EXPANDER_MAP.csv")
        m100 = [r for r in rows if r["Module"] == "M100 PRO GPS assembly"]
        qmc = [r for r in rows if r["Module"] == "M100 PRO integrated QMC compass"]
        self.assertEqual(len(m100), 1)
        self.assertEqual(len(qmc), 1)
        self.assertEqual(m100[0]["Quantity"], "1")
        self.assertEqual(qmc[0]["Quantity"], "0")
        self.assertEqual(qmc[0]["State"], "INCLUDED_WITH_M100_PRO")
        self.assertIn("3.6-5.5V", m100[0]["Commissioning_Blocker"])

    def test_5v_levels_are_unverified_and_not_energized(self):
        note = (ROOT / "Boat/docs/BOAT_R73_M100_PRO_INTEGRATED_COMPASS_POWER.md").read_text(encoding="utf-8")
        for phrase in ("3.6–5.5 V", "I²C", "UART", "3.3 V",
                       "used to power the M100 PRO",
                       "level-shifting", "No new hardware purchase"):
            self.assertIn(phrase.casefold(), note.casefold())
        sketch = (ROOT / "Boat/source/firmware/Flytec_Boat.ino").read_text(encoding="utf-8")
        self.assertNotIn("application.attach(", sketch)
        self.assertEqual((ROOT / "Boat/releases/VERSION").read_text().strip(),"014")
        self.assertEqual((ROOT / "Remote/releases/VERSION").read_text().strip(),"028")

if __name__ == "__main__":
    unittest.main()
