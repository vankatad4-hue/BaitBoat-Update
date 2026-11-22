"""R20 static DevKitC-1 N32R16V GPIO/header candidate check.

This validates the documented candidate crosswalk only. It is NOT a physical
board revision audit, electrical test or actuator safety acceptance.
"""
import csv
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
CROSSWALK = ROOT / "Boat/docs/BOAT_R20_GPIO_RESOURCE_CROSSWALK.csv"

EXPECTED_ROLES = {
    "GNSS_TX_TO_MODULE_RX": ("J1", "4", "GPIO4"),
    "GNSS_RX_FROM_MODULE_TX": ("J1", "5", "GPIO5"),
    "E22_NRST_CANDIDATE": ("J1", "6", "GPIO6"),
    "I2C_SCL_CANDIDATE": ("J1", "7", "GPIO7"),
    "E22_M0_CANDIDATE": ("J1", "8", "GPIO15"),
    "E22_M1_CANDIDATE": ("J1", "9", "GPIO16"),
    "E22_TX_CANDIDATE": ("J1", "10", "GPIO17"),
    "E22_RX_CANDIDATE": ("J1", "11", "GPIO18"),
    "I2C_SDA_CANDIDATE": ("J1", "12", "GPIO8"),
    "NRF_CSN_CANDIDATE": ("J1", "15", "GPIO9"),
    "NRF_CE_CANDIDATE": ("J1", "16", "GPIO10"),
    "NRF_MOSI_CANDIDATE": ("J1", "17", "GPIO11"),
    "NRF_SCK_CANDIDATE": ("J1", "18", "GPIO12"),
    "NRF_MISO_CANDIDATE": ("J1", "19", "GPIO13"),
    "E22_AUX_CANDIDATE": ("J1", "20", "GPIO14"),
    "MDD20A_PWM2_CANDIDATE": ("J3", "8", "GPIO40"),
    "MDD20A_PWM1_CANDIDATE": ("J3", "9", "GPIO39"),
    "NRF_IRQ_CANDIDATE": ("J3", "18", "GPIO21"),
}
NEVER_ASSIGN = {
    "GPIO0", "GPIO3", "GPIO19", "GPIO20", "GPIO35", "GPIO36",
    "GPIO37", "GPIO38", "GPIO43", "GPIO44", "GPIO45", "GPIO46",
    "GPIO47", "GPIO48",
}
RADIO_CANDIDATES = {
    "BOAT_NRF_CE_PIN": 10,
    "BOAT_NRF_CSN_PIN": 9,
    "BOAT_NRF_SCK_PIN": 12,
    "BOAT_NRF_MISO_PIN": 13,
    "BOAT_NRF_MOSI_PIN": 11,
    "BOAT_NRF_IRQ_PIN": 21,
    "BOAT_E22_UART_PIN_TX": 17,
    "BOAT_E22_UART_PIN_RX": 18,
    "BOAT_E22_UART_PIN_M0": 15,
    "BOAT_E22_UART_PIN_M1": 16,
    "BOAT_E22_UART_PIN_AUX": 14,
    "BOAT_E22_UART_PIN_NRST": 6,
}
RADIO_HOLDS = (
    "BOAT_NRF_PIN_AUDIT_SIGNED",
    "BOAT_E22_UART_PIN_AUDIT_SIGNED",
    "BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED",
    "BOAT_E22_UART_LOGIC_LEVELS_VERIFIED",
)

class BoatR20PinResources(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with CROSSWALK.open(encoding="utf-8", newline="") as f:
            cls.rows = list(csv.DictReader(f))

    def test_complete_J1_J3_44_header(self):
        self.assertEqual(len(self.rows), 44)
        header_positions = {(r["Header"], r["Pin"]) for r in self.rows}
        expected = {(h, str(i)) for h in ("J1", "J3") for i in range(1, 23)}
        self.assertEqual(header_positions, expected)

    def test_role_uniqueness_and_no_reserved_gpio(self):
        assigned = [r for r in self.rows if r["Proposed_Logical_Function"] != "NO_ASSIGN"]
        self.assertEqual(len(assigned), len(EXPECTED_ROLES))
        self.assertEqual({r["Proposed_Logical_Function"] for r in assigned}, set(EXPECTED_ROLES))
        self.assertEqual(len({r["Signal"] for r in assigned}), len(assigned))
        for r in self.rows:
            if r["Signal"] in NEVER_ASSIGN:
                self.assertEqual(r["Proposed_Logical_Function"], "NO_ASSIGN", r["Signal"])
            if r["Proposed_Logical_Function"] != "NO_ASSIGN":
                self.assertEqual(r["Voltage_Domain"], "3V3")
                self.assertTrue(r["Signal"].startswith("GPIO"))
                self.assertEqual(
                    (r["Header"], r["Pin"], r["Signal"]),
                    EXPECTED_ROLES[r["Proposed_Logical_Function"]]
                )

    def test_frozen_1v8_and_memory_pins(self):
        lookup = {r["Signal"]: r for r in self.rows if r["Signal"].startswith("GPIO")}
        for signal in ("GPIO47", "GPIO48"):
            self.assertEqual(lookup[signal]["Voltage_Domain"], "1V8")
        for signal in ("GPIO35", "GPIO36", "GPIO37"):
            self.assertEqual(lookup[signal]["Constraint"], "OCTAL_MEMORY_RESERVED")

    def test_release_requests_stay_on_mcp23017_not_direct_esp_gpio(self):
        roles = [r["Proposed_Logical_Function"] for r in self.rows]
        self.assertFalse(any("EKM014" in role or "SOLENOID_" in role for role in roles))
        doc = (ROOT / "Boat/docs/BOAT_GPIO_HEADER_MAP_R20.md").read_text()
        adapter = (ROOT / "Boat/source/ekm014_mcp23017_adapter.h").read_text()
        physical = (ROOT / "Boat/source/boat_physical_driver_adapter.h").read_text()
        self.assertIn("MCP23017 GPA0", doc)
        self.assertIn("does NOT replace Ekm014PulseGuard", adapter)
        self.assertIn("independentHardwareCommissioned", physical)

    def test_hardware_still_uncommissioned(self):
        config = (ROOT / "Boat/source/boat_radio_hw_config.h").read_text()
        for macro, value in RADIO_CANDIDATES.items():
            self.assertRegex(config, rf"(?m)^#define {macro} {value}$")
        for macro in RADIO_HOLDS:
            self.assertRegex(config, rf"(?m)^#define {macro} 0$")
        profile = (ROOT / "Boat/source/tracker_sensor_profile.example.h").read_text()
        self.assertIn("pinAuditSigned=false", profile)
        self.assertIn("c.battery={false,", profile)
        sketch = (ROOT / "Boat/source/firmware/Flytec_Boat.ino").read_text()
        self.assertIn("application.tick(); // no commissioned driver attached", sketch)
        self.assertNotIn("application.attach(", sketch)
        self.assertEqual((ROOT / "Boat/releases/VERSION").read_text().strip(), "014")
        self.assertEqual((ROOT / "Remote/releases/VERSION").read_text().strip(), "028")

if __name__ == "__main__":
    unittest.main()
