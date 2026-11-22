#!/usr/bin/env python3
"""R30H-1 hostile mutation checks; host source audit, no hardware signoff."""
import importlib.util
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("r30h_preflight", ROOT / "Boat/tools/verify_r30h_radio_preflight.py")
check = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(check)

class TestR30HPreflight(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.src = check.snapshot()

    def test_snapshot_passes_unsigned(self):
        self.assertEqual([], check.verify(self.src))

    def hostile(self, path, before, after):
        mutated = dict(self.src)
        self.assertIn(before, mutated[path])
        mutated[path] = mutated[path].replace(before, after, 1)
        self.assertTrue(check.verify(mutated), f"mutated {path} was accepted")

    def test_fail_if_inverted_txlvl_returns(self):
        self.hostile("Remote/source/sc16is750_bridge.cpp",
                     "freeBytes=r30h_sc16::freeTxSlots(level)", "freeBytes=64U-level")

    def test_fail_if_remote_radio_unlocked(self):
        self.hostile("Remote/source/radio_hw_config.h",
                     "#define REMOTE_NRF_PIN_AUDIT_SIGNED 0",
                     "#define REMOTE_NRF_PIN_AUDIT_SIGNED 1")

    def test_fail_if_boat_airtime_unsigned_gate_removed(self):
        self.hostile("Boat/source/boat_dual_radio_port.cpp",
                     "&& BOAT_E22_RF_AIRTIME_AUDIT_SIGNED", "&& true")

    def test_fail_if_r30h2_local_trace_unlocked(self):
        self.hostile("Boat/source/boat_radio_hw_config.h",
                     "#define BOAT_R30H_LOCAL_TRACE_ENABLED 0",
                     "#define BOAT_R30H_LOCAL_TRACE_ENABLED 1")

    def test_fail_if_remote_r30h3_trace_unlocked(self):
        self.hostile("Remote/source/radio_hw_config.h",
                     "#define REMOTE_R30H3_STOP_TRACE_ENABLED 0",
                     "#define REMOTE_R30H3_STOP_TRACE_ENABLED 1")

    def test_fail_if_e22_profiles_diverge(self):
        self.hostile("Remote/source/e22_uart_link.cpp",
                     "0x67,0xC3,0x12", "0x67,0xC3,0x13")

    def test_fail_if_fake_physical_signoff(self):
        self.hostile("Boat/docs/BOAT_R30H_1_RF_MODULE_SIGNOFF.csv",
                     "NOT_RUN,,,", "PASS,dummy,not_verified,")

    def test_fail_if_motor_driver_attaches(self):
        self.hostile("Boat/source/firmware/Flytec_Boat.ino",
                     "const bool ready=radio.begin();",
                     "application.attach(fakeDriver,fakeConfig);\n const bool ready=radio.begin();")

if __name__ == "__main__":
    unittest.main()
