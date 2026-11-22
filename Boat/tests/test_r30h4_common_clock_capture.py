#!/usr/bin/env python3
"""Host-only hostile-input regressions for R30H-4 external clock parser."""
import hashlib
import importlib.util
import json
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "r30h4_parser", ROOT / "Boat/tools/analyze_r30h4_common_clock_capture.py")
api = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(api)

HEADER = "capture_id,trial_id,case_id,clock_domain,time_us,platform,event,edge,origin,notes\n"

def edge(t, event, platform, side, case="H05", trial="T01", clock="LA_UNIT", origin="SYNTHETIC"):
    return f"C01,{trial},{case},{clock},{t},{platform},{event},{side},{origin},\n"

def manifest(mode="SYNTHETIC", clock="LA_UNIT"):
    return {"schema":"R30H4-1", "evidence_mode":mode, "clock_domain":clock,
            "capture_file_sha256":"",
            "instrument":{"model":"","asset_id":"","sample_rate_hz":0},
            "firmware":{"remote_commit":"","boat_commit":""},
            "hardware":{"actuator_power_disconnected":None},
            "physical_preflight_evidence":""}

class CommonClockTests(unittest.TestCase):
    def read(self, contents):
        with tempfile.TemporaryDirectory() as tmp:
            p=pathlib.Path(tmp)/"edges.csv"
            p.write_text(contents,encoding="utf-8")
            return api.parse_csv(p)

    def test_empty_templates_are_not_run(self):
        template = ROOT / "Boat/docs/BOAT_R30H_4_LOGIC_CAPTURE_TEMPLATE.csv"
        m = ROOT / "Boat/docs/BOAT_R30H_4_CAPTURE_MANIFEST_TEMPLATE.json"
        self.assertEqual({}, dict(api.parse_csv(template)))
        self.assertEqual("UNSPECIFIED", api.read_manifest(m)["evidence_mode"])
        self.assertEqual(3, api.main([str(template),str(m)]))

    def test_synthetic_uart_path_is_review_only_even_with_complete_edges(self):
        records=self.read(HEADER +
                          edge(100, "STOP_BUTTON_EDGE","REMOTE","RISING")+
                          edge(3100, "REMOTE_E22_UART_TX_START","REMOTE","FALLING")+
                          edge(3200, "BOAT_E22_UART_RX_START","BOAT","FALLING"))
        self.assertEqual([],api.validate_manifest(manifest(), pathlib.Path("/dev/null"),records))
        errors, metrics, incomplete=api.evaluate(records)
        self.assertFalse(errors)
        self.assertEqual(3.0, metrics[("H05","button_to_remote_e22_uart_ms")][0])
        self.assertEqual(0.1, metrics[("H05","e22_uart_tx_to_rx_ms")][0])
        self.assertEqual(0,incomplete)

    def test_missing_boat_uart_edge_not_physical_stop(self):
        rows=self.read(HEADER +
                       edge(100, "STOP_BUTTON_EDGE","REMOTE","RISING")+
                       edge(3000, "REMOTE_E22_UART_TX_START","REMOTE","FALLING"))
        errors, metrics, incomplete=api.evaluate(rows)
        self.assertEqual([],errors)
        self.assertEqual(1,incomplete)
        self.assertNotIn(("H05","e22_uart_tx_to_rx_ms"),metrics)

    def test_mixed_timing_sources_rejected(self):
        with self.assertRaisesRegex(ValueError,"origin"):
            self.read(HEADER + edge(100,"STOP_BUTTON_EDGE","REMOTE","RISING",origin="USB_LOCAL"))
        records=self.read(HEADER +
                          edge(100,"STOP_BUTTON_EDGE","REMOTE","RISING")+
                          edge(3000,"REMOTE_E22_UART_TX_START","REMOTE","FALLING",origin="EXTERNAL_ANALYZER"))
        self.assertTrue(api.validate_manifest(manifest(),pathlib.Path("/dev/null"),records))

    def test_mixed_clock_rejected(self):
        with self.assertRaisesRegex(ValueError,"mixed clock_domain"):
            self.read(HEADER +
                      edge(100,"STOP_BUTTON_EDGE","REMOTE","RISING")+
                      edge(200,"REMOTE_E22_UART_TX_START","REMOTE","FALLING",clock="LA2"))

    def test_unsigned_switch_and_rf_fixture_edge_polarity_not_assumed(self):
        records = self.read(HEADER +
            edge(100, "STOP_BUTTON_EDGE", "REMOTE", "FALLING", case="H04") +
            edge(200, "RF_CUT_TRIGGER", "RF_FIXTURE", "FALLING", case="H08", trial="T02"))
        self.assertEqual(2, len(records))
        with self.assertRaisesRegex(ValueError, "event/platform/edge"):
            self.read(HEADER + edge(100, "REMOTE_E22_UART_TX_START",
                                    "REMOTE", "RISING"))

    def test_reversed_edges_and_invalid_identity_rejected(self):
        with self.assertRaisesRegex(ValueError,"monotonically"):
            self.read(HEADER + edge(500,"STOP_BUTTON_EDGE","REMOTE","RISING")+
                      edge(100,"REMOTE_E22_UART_TX_START","REMOTE","FALLING"))
        with self.assertRaisesRegex(ValueError,"event/platform/edge"):
            self.read(HEADER + edge(100,"BOAT_E22_UART_RX_START","REMOTE","RISING"))

    def test_duplicate_event_fails(self):
        with self.assertRaisesRegex(ValueError,"duplicate"):
            self.read(HEADER + edge(100,"STOP_BUTTON_EDGE","REMOTE","RISING")+
                      edge(200,"STOP_BUTTON_EDGE","REMOTE","RISING"))

    def test_rth_9999999us_fails_and_exact_10000000us_is_only_review(self):
        early=self.read(HEADER +
                        edge(100000,"BOAT_LAST_AUTH_RX_MARKER","BOAT","RISING",case="H11")+
                        edge(10099999,"BOAT_RTH_ENTRY_MARKER","BOAT","RISING",case="H11"))
        problems, _, _=api.evaluate(early)
        self.assertTrue(any("earlier" in p for p in problems))
        exact=self.read(HEADER +
                        edge(100000,"BOAT_LAST_AUTH_RX_MARKER","BOAT","RISING",case="H11")+
                        edge(10100000,"BOAT_RTH_ENTRY_MARKER","BOAT","RISING",case="H11"))
        problems, metrics, _=api.evaluate(exact)
        self.assertEqual([],problems)
        self.assertEqual([10000.0], metrics[("H11","last_auth_marker_to_rth_marker_ms")])

    def test_unclaimed_hardware_evidence_is_not_permitted(self):
        rows=self.read(HEADER + edge(100,"STOP_BUTTON_EDGE","REMOTE","RISING",origin="EXTERNAL_ANALYZER"))
        errors=api.validate_manifest(manifest("HARDWARE_OBSERVATION"),pathlib.Path("/dev/null"),rows)
        self.assertTrue(any("SHA256" in x for x in errors))
        self.assertTrue(any("VM/VH" in x for x in errors))
        self.assertTrue(any("sample rate" in x for x in errors))

    def test_full_hardware_metadata_still_returns_review_required_not_pass(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder=pathlib.Path(tmp)
            capture=folder/"real.csv"; capture.write_text(
                HEADER + edge(100,"STOP_BUTTON_EDGE","REMOTE","RISING",origin="EXTERNAL_ANALYZER")+
                edge(200,"REMOTE_E22_UART_TX_START","REMOTE","FALLING",origin="EXTERNAL_ANALYZER"), encoding="utf-8")
            data=manifest("HARDWARE_OBSERVATION")
            data["capture_file_sha256"]=hashlib.sha256(capture.read_bytes()).hexdigest()
            data["instrument"]={"model":"fixture-logic-analyzer","asset_id":"UNIT-TEST","sample_rate_hz":1000000}
            data["firmware"]={"remote_commit":"0"*40,"boat_commit":"1"*40}
            data["hardware"]={"remote_board_revision":"SIMULATION","boat_board_revision":"SIMULATION",
                              "e22_remote_sku":"SIM","e22_boat_sku":"SIM",
                              "actuator_power_disconnected":True}
            data["physical_preflight_evidence"]="UNIT-TEST-NOT_REAL"
            self.assertEqual([],api.validate_manifest(data,capture,api.parse_csv(capture)))
            m=folder/"metadata.json";m.write_text(json.dumps(data),encoding="utf-8")
            # 4 is a non-success exit and human review marker even with forged metadata.
            self.assertEqual(4,api.main([str(capture),str(m)]))

    def test_zero_data_with_unsafe_manifest_never_becomes_signed(self):
        self.assertEqual([],api.validate_manifest(
            manifest("HARDWARE_OBSERVATION"),
            pathlib.Path("/tmp/does-not-exist.csv"), {}))


if __name__=="__main__":
    unittest.main()
