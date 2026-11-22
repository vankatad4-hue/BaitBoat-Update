#!/usr/bin/env python3
"""R30H-3 synthetic scheduling tests. These are not RF propagation measurements."""
import importlib.util
from dataclasses import replace
from pathlib import Path
import unittest

P = Path(__file__).resolve().parents[1] / "tools/simulate_r30h3_rf_timing.py"
SPEC = importlib.util.spec_from_file_location("r30h3_exposure", P)
mod = importlib.util.module_from_spec(SPEC)
import sys
sys.modules[SPEC.name] = mod
SPEC.loader.exec_module(mod)


class TestR30H3SoftwareOnly(unittest.TestCase):
    def test_current_source_boundaries(self):
        p = mod.POLICY
        self.assertEqual((250, 6000, 3000, 10000),
                         (p.nrf_fresh_ms, p.e22_fresh_ms,
                          p.e22_pace_ms, p.rth_after_ms))

    def test_all_e22_heartbeat_phases_enumerated(self):
        e = mod.exposure(mod.POLICY)
        self.assertEqual({"min": 0, "max": 3000,
                          "examples": {"just_after_tx": 3000, "just_before_next_slot": 1, "at_next_slot": 0}},
                         e["remote_e22_stop_queue_wait_ms"])
        self.assertEqual(251, e["nrf_only_first_stale_after_cut_ms"])
        self.assertEqual(6001, e["e22_only_first_stale_after_cut_ms"])
        self.assertEqual({"min": 3002, "max": 6001}, e["both_links_stale_after_cut_ms"])
        self.assertEqual(10000, e["earliest_rth_after_last_auth_ms"])
        self.assertIsNone(e["real_rf_or_physical_output_latency_ms"])

    def test_steady_state_heartbeat_has_no_command_headroom_if_airtime_30ms(self):
        b = mod.hypothetical_budget(mod.POLICY, 30)
        self.assertEqual(1200, b["periodic_frames_per_hour_per_endpoint_if_no_drops"])
        self.assertEqual(36000, b["periodic_airtime_ms_per_hour"])
        self.assertEqual(0, b["provisional_remaining_ms_for_commands"])
        self.assertIn("HYPOTHETICAL", b["status"])
        more = mod.hypothetical_budget(mod.POLICY, 50)
        self.assertLess(more["provisional_remaining_ms_for_commands"], 0)

    def test_empty_airtime_has_no_measured_value(self):
        self.assertEqual("NOT_MEASURED",
                         mod.hypothetical_budget(mod.POLICY, None)["status"])
        for invalid in (0, -1, float("inf"), float("nan")):
            with self.assertRaises(ValueError):
                mod.hypothetical_budget(mod.POLICY, invalid)

    def test_rth_timing_is_distinct_from_watchdog(self):
        e = mod.exposure(mod.POLICY)
        self.assertGreater(e["earliest_rth_after_last_auth_ms"],
                           e["both_links_stale_after_cut_ms"]["max"])
        self.assertEqual(0, mod.remote_stop_wait_ms(-4000, 0, 3000))
        self.assertEqual(1, mod.remote_stop_wait_ms(0, 2999, 3000))

    def test_adversarial_shrinking_or_mismatched_source_detected(self):
        original = mod.POLICY
        self.assertEqual(10000, original.rth_after_ms)
        # A counterfactual shorter RTH threshold is visible as a policy change;
        # no "PASS" signoff can be inferred from any returned number.
        altered = mod.exposure(replace(original, rth_after_ms=9000))
        self.assertEqual("DETERMINISTIC_SOFTWARE_ONLY_NO_RF", altered["scenario"])
        self.assertEqual(9000, altered["earliest_rth_after_last_auth_ms"])


if __name__ == "__main__":
    unittest.main()
