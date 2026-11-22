#!/usr/bin/env python3
"""Regression of R30H event-only analyzer; NOT a physical hardware test."""
import importlib.util
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "Boat/tools/analyze_r30h_rf_events.py"
spec = importlib.util.spec_from_file_location("r30h", SCRIPT)
analyzer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(analyzer)


class R30HAnalyzerTests(unittest.TestCase):
    def test_empty_template_is_not_run(self):
        p = ROOT / "Boat/docs/BOAT_R30H_RF_EVENTS_TEMPLATE.csv"
        self.assertEqual({}, dict(analyzer.parse(p)))

    def test_stop_measures_both_transport_and_logic(self):
        events = {"INPUT_STOP": 100000, "AUTH_STOP_RX": 140000,
                  "STOP_INTENT": 146000, "LOGICAL_OUTPUT_OFF": 149000}
        errors, metrics, *_ = analyzer.check(
            "001", {"meta": ("H04", "NRF", "LA1"), "events": events}, 100.0)
        self.assertEqual([], errors)
        self.assertEqual(49.0, metrics["stop_e2e_logic_ms"])
        errors, _, *_ = analyzer.check(
            "001", {"meta": ("H04", "NRF", "LA1"), "events": events}, 40.0)
        self.assertTrue(errors)

    def test_early_rth_fail_closed(self):
        basis = {"RF_BLOCKED": 200000, "LAST_AUTH_RX": 190000,
                 "BOTH_STALE": 6500000, "LOGICAL_OUTPUT_OFF": 6502000}
        early = {**basis, "RTH_ENTRY": 10189000}
        errors, *_ = analyzer.check("001", {"meta": ("H11", "DUAL", "LA1"), "events": early})
        self.assertTrue(any("EARLY" in e for e in errors))
        ontime = {**basis, "RTH_ENTRY": 10190000}
        errors, metrics, *_ = analyzer.check("002", {"meta": ("H11", "DUAL", "LA1"), "events": ontime})
        self.assertFalse(errors)
        self.assertEqual(10000.0, metrics["last_auth_to_rth_ms"])

    def test_invalid_home_rejects_rth(self):
        events = {"RF_BLOCKED": 200000, "LAST_AUTH_RX": 190000,
                  "BOTH_STALE": 6500000, "LOGICAL_OUTPUT_OFF": 6502000,
                  "RTH_ENTRY": 10300000}
        errors, *_ = analyzer.check("001", {"meta": ("H13", "DUAL", "LA1"), "events": events})
        self.assertTrue(any("forbidden RTH" in e for e in errors))

    def test_traces_never_mix_clocks_or_repeat_events(self):
        header = "trial_id,case_id,transport,clock_domain,platform,event,time_us,notes\n"
        with tempfile.TemporaryDirectory() as d:
            p = pathlib.Path(d) / "log.csv"
            p.write_text(header +
                         "A,H04,NRF,LA1,Remote,INPUT_STOP,10,\n"
                         "A,H04,NRF,LA2,Boat,AUTH_STOP_RX,20,\n",
                         encoding="utf-8")
            with self.assertRaises(ValueError):
                analyzer.parse(p)
            p.write_text(header +
                         "A,H04,NRF,LA1,Remote,INPUT_STOP,10,\n"
                         "A,H04,NRF,LA1,Remote,INPUT_STOP,20,\n",
                         encoding="utf-8")
            with self.assertRaises(ValueError):
                analyzer.parse(p)

    def test_unsupported_case_requires_human_review(self):
        errors, metrics, case, transport, clock, assessed = analyzer.check(
            "001", {"meta": ("H18", "DUAL", "LA1"), "events": {"AUTH_LINK_RESTORED": 1234}})
        self.assertFalse(assessed)
        self.assertFalse(errors)
        self.assertEqual({}, metrics)

    def test_no_rth_without_evidence_and_release_activity_fail(self):
        events = {"RF_BLOCKED": 200000, "LAST_AUTH_RX": 190000,
                  "BOTH_STALE": 6500000, "LOGICAL_OUTPUT_OFF": 6502000,
                  "RELEASE_ACTIVE": 7000000}
        errors, *_ = analyzer.check("001", {"meta": ("H11", "DUAL", "LA1"), "events": events})
        self.assertTrue(any("EARLY" in e for e in errors))
        self.assertTrue(any("release active" in e for e in errors))


if __name__ == "__main__":
    unittest.main()
