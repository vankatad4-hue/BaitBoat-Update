#!/usr/bin/env python3
"""R30H-2 input validation tests; entirely simulated samples, never RF."""
import importlib.util
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location(
    "r30h2_local", ROOT / "Boat/tools/analyze_r30h2_boat_local_trace.py")
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)


def rec(ms, last, event, link=0, mask=0, cmd=0, drops=0):
    return f"R30H2,BOAT_LOCAL,{ms},{last},{event},{link},{mask},{cmd},{drops},INTENT_ONLY\n"


class TestR30H2BoatTrace(unittest.TestCase):
    def test_empty_is_not_run(self):
        self.assertEqual(("NOT_RUN", [], {}), mod.analyze(mod.parse([])))

    def test_valid_stop_local_processing_only(self):
        text = "R30E,LOCAL_ONLY,ignored\n" + rec(100, 100, "STOP_AUTH_RECEIVED", 1, 0, 18) + rec(102, 100, "STOP_LOGICAL_INTENT", 1, 0, 18)
        result, errors, metric = mod.analyze(mod.parse(text.splitlines()))
        self.assertEqual("BOAT_LOCAL_OBSERVATIONS_ONLY", result)
        self.assertFalse(errors)
        self.assertEqual([2], metric["stop_auth_to_intent_ms"])

    def test_valid_rth_at_ten_seconds(self):
        rows = mod.parse((rec(6100, 0, "BOTH_LINKS_STALE") +
                          rec(10000, 0, "RTH_LOCAL_ENTRY")).splitlines())
        result, errors, metric = mod.analyze(rows)
        self.assertEqual("BOAT_LOCAL_OBSERVATIONS_ONLY", result)
        self.assertEqual([], errors)
        self.assertEqual([10000], metric["last_auth_to_rth_ms"])

    def test_rth_one_ms_early_is_rejected(self):
        rows = mod.parse((rec(6100, 0, "BOTH_LINKS_STALE") +
                          rec(9999, 0, "RTH_LOCAL_ENTRY")).splitlines())
        result, errors, _ = mod.analyze(rows)
        self.assertEqual("INVALID_OR_INCOMPLETE", result)
        self.assertTrue(any("EARLY" in err for err in errors))

    def test_recovered_link_forbids_unsignaled_rth(self):
        rows = mod.parse((rec(6100, 0, "BOTH_LINKS_STALE") +
                          rec(7000, 7000, "AUTH_LINK_RESTORED", 2, 2) +
                          rec(18000, 7000, "RTH_LOCAL_ENTRY")).splitlines())
        _, errors, _ = mod.analyze(rows)
        self.assertTrue(any("without recorded both-links-stale" in err for err in errors))

    def test_lost_stop_intent_is_not_success(self):
        rows = mod.parse([rec(1000, 1000, "STOP_AUTH_RECEIVED", 2, 0, 33)])
        result, errors, _ = mod.analyze(rows)
        self.assertEqual("INVALID_OR_INCOMPLETE", result)
        self.assertTrue(any("missing logical STOP" in err for err in errors))

    def test_drop_or_fake_physical_label_is_never_accepted(self):
        rows = mod.parse([rec(1000, 1000, "STOP_AUTH_RECEIVED", 1, 0, 33, 1),
                          rec(1001, 1000, "STOP_LOGICAL_INTENT", 1, 0, 33, 1)])
        result, errors, _ = mod.analyze(rows)
        self.assertEqual("INVALID_OR_INCOMPLETE", result)
        self.assertTrue(any("overflow" in err for err in errors))
        with self.assertRaises(ValueError):
            mod.parse([rec(1000, 1000, "STOP_AUTH_RECEIVED", 1, 0, 33).replace("INTENT_ONLY", "CURRENT_ZERO")])

    def test_single_clock_rollover(self):
        rows = mod.parse([rec(4294967294, 4294967294, "STOP_AUTH_RECEIVED", 1, 0, 9),
                          rec(2, 4294967294, "STOP_LOGICAL_INTENT", 1, 0, 9)])
        _, errors, metric = mod.analyze(rows)
        self.assertFalse(errors)
        self.assertEqual([4], metric["stop_auth_to_intent_ms"])

    def test_reversed_millis_rejected(self):
        rows = mod.parse([rec(300, 300, "STOP_AUTH_RECEIVED", 1, 0, 9),
                          rec(250, 300, "STOP_LOGICAL_INTENT", 1, 0, 9)])
        _, errors, _ = mod.analyze(rows)
        self.assertTrue(any("out of order" in err or "precedes" in err for err in errors))


if __name__ == "__main__":
    unittest.main()
