#!/usr/bin/env python3
"""Static diagnostic lifecycle guard. Not a substitute for target tests."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
class DualCoreContract(unittest.TestCase):
    def test_matching_boat_remote(self):
        self.assertEqual(
            (ROOT / "Boat/source/dual_core_diagnostic.h").read_bytes(),
            (ROOT / "Remote/source/dual_core_diagnostic.h").read_bytes())
    def test_compile_only_units(self):
        for side in ("Boat", "Remote"):
            unit = (ROOT / side / "source/dual_core_diagnostic_compile.cpp").read_text()
            self.assertIn('#include "dual_core_diagnostic.h"', unit)
            self.assertNotIn("xTaskCreatePinnedToCore(", unit)
            self.assertNotIn("setup()", unit)
    def test_pinning_and_lifecycle(self):
        source = (ROOT / "Boat/source/dual_core_diagnostic.h").read_text()
        self.assertIn('nullptr, 0) != pdPASS', source)
        self.assertIn('nullptr, 1) != pdPASS', source)
        self.assertIn('std::atomic<bool> done0_', source)
        self.assertIn('waitForExit();', source)
        self.assertNotIn('TaskHandle_t handle', source)
        self.assertNotIn('xTaskNotifyGive', source)
        self.assertNotIn('digitalWrite(', source)
        self.assertNotIn('xQueueSend(', source)
if __name__ == "__main__":
    unittest.main()
