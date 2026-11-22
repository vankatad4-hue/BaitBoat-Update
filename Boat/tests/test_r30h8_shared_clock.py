#!/usr/bin/env python3
"""R30H-8 synthetic PPS/single-instrument clock-offset fixtures, NO_REAL_GPS."""
import hashlib
import importlib.util
import json
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
S = importlib.util.spec_from_file_location(
    "r30h8", ROOT / "Boat/tools/analyze_r30h8_shared_clock.py")
mod = importlib.util.module_from_spec(S)
S.loader.exec_module(mod)

HEADER = "capture_id,trial_id,clock_domain,time_us,platform,event,utc_unix_ms,origin,notes\n"
EPOCH = 1795347296000

def edge(trial, t, platform, event, utc, origin="SYNTHETIC", clock="ANALYZER_SIM"):
    return f"C01,{trial},{clock},{t},{platform},{event},{utc},{origin},\n"

def rows(origin="SYNTHETIC"):
    return (HEADER +
            edge("T01", 1000000, "GNSS", "GNSS_PPS_EDGE", EPOCH, origin) +
            edge("T01", 1100000, "BOAT", "BOAT_RTC_SAMPLE_EDGE", EPOCH+100, origin) +
            edge("T01", 1200000, "REMOTE", "REMOTE_RTC_SAMPLE_EDGE", EPOCH+201, origin) +
            edge("T02", 2000000, "GNSS", "GNSS_PPS_EDGE", EPOCH+1000, origin) +
            edge("T02", 2100000, "BOAT", "BOAT_RTC_SAMPLE_EDGE", EPOCH+1100, origin) +
            edge("T02", 2200000, "REMOTE", "REMOTE_RTC_SAMPLE_EDGE", EPOCH+1201, origin))

def manifest(mode="SYNTHETIC", clock="ANALYZER_SIM"):
    return {"schema":"R30H8-1","evidence_mode":mode,"clock_domain":clock}

class TestR30H8(unittest.TestCase):
    def test_blank_templates_remain_not_run(self):
        root = ROOT / "Boat/docs"
        self.assertEqual(3, mod.main([
            str(root / "BOAT_R30H_8_SHARED_CLOCK_CAPTURE_TEMPLATE.csv"),
            str(root / "BOAT_R30H_8_SHARED_CLOCK_MANIFEST_TEMPLATE.json")]))
    
    def test_simulated_integer_offsets_and_drift(self):
        with tempfile.TemporaryDirectory() as td:
            p = pathlib.Path(td)/"capture.csv"
            p.write_text(rows(),encoding="utf-8")
            trials,origins,clocks = mod.parse_csv(p)
            self.assertEqual({"SYNTHETIC"},origins)
            self.assertEqual({"ANALYZER_SIM"},clocks)
            self.assertEqual([],mod.validate_provenance(manifest(),p,trials,origins,clocks))
            errors,records = mod.compute(trials)
            self.assertEqual([],errors)
            self.assertEqual([0,0],[r["boat_offset_us"] for r in records])
            self.assertEqual([1000,1000],[r["remote_offset_us"] for r in records])
            self.assertEqual(1000,mod.describe(records)["offset_candidates_us"]["remote_minus_boat_us"]["max"])
            self.assertEqual(0,mod.describe(records)["drift_candidates_ppm"]["C01/boat_ppm_candidate"])
            m = pathlib.Path(td)/"manifest.json"
            m.write_text(json.dumps(manifest()),encoding="utf-8")
            self.assertEqual(4,mod.main([str(p),str(m)]))  # NEVER a PASS

    def test_usb_local_and_mixed_origins_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            p=pathlib.Path(td)/"bad.csv"
            p.write_text(rows().replace("SYNTHETIC","USB_LOCAL"),encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "USB"):
                mod.parse_csv(p)
            p.write_text(rows().replace("SYNTHETIC","EXTERNAL_ANALYZER",1),encoding="utf-8")
            trials,origins,clocks=mod.parse_csv(p)
            self.assertTrue(mod.validate_provenance(manifest(),p,trials,origins,clocks))

    def test_missing_remote_marker_no_offset_claim(self):
        with tempfile.TemporaryDirectory() as td:
            p=pathlib.Path(td)/"missing.csv"
            p.write_text(rows().replace(edge("T02",2200000,"REMOTE","REMOTE_RTC_SAMPLE_EDGE",EPOCH+1201),""),encoding="utf-8")
            parsed,_,_=mod.parse_csv(p)
            errors,obs=mod.compute(parsed)
            self.assertTrue(any("both clock sample" in e for e in errors))
            self.assertEqual(1,len(obs))

    def test_pps_must_mark_whole_unix_second(self):
        with tempfile.TemporaryDirectory() as td:
            p=pathlib.Path(td)/"bad.csv"
            p.write_text(rows().replace(
                edge("T01",1000000,"GNSS","GNSS_PPS_EDGE",EPOCH),
                edge("T01",1000000,"GNSS","GNSS_PPS_EDGE",EPOCH+1)),encoding="utf-8")
            with self.assertRaisesRegex(ValueError,"whole UTC second"):
                mod.parse_csv(p)

    def test_marker_must_be_in_same_pps_epoch(self):
        with tempfile.TemporaryDirectory() as td:
            p=pathlib.Path(td)/"bad.csv"
            p.write_text(rows().replace(
                edge("T01",1200000,"REMOTE","REMOTE_RTC_SAMPLE_EDGE",EPOCH+201),
                edge("T01",1999999,"REMOTE","REMOTE_RTC_SAMPLE_EDGE",EPOCH+201)),encoding="utf-8")
            tr,_,_=mod.parse_csv(p)
            errors,_=mod.compute(tr)
            self.assertFalse(errors)  # still within same second; biased source is only candidate
            p.write_text(rows().replace(
                edge("T01",1200000,"REMOTE","REMOTE_RTC_SAMPLE_EDGE",EPOCH+201),
                edge("T01",2000000,"REMOTE","REMOTE_RTC_SAMPLE_EDGE",EPOCH+201)),encoding="utf-8")
            with self.assertRaisesRegex(ValueError,"out of order"):
                mod.parse_csv(p)  # source capture edge order strictly enforced

    def test_duplicate_sample_and_wrong_platform_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            p=pathlib.Path(td)/"bad.csv"
            original=rows()
            p.write_text(original + edge("T02",2300000,"REMOTE","REMOTE_RTC_SAMPLE_EDGE",EPOCH+1300),encoding="utf-8")
            with self.assertRaisesRegex(ValueError,"duplicate"):
                mod.parse_csv(p)
            p.write_text(original.replace("GNSS,GNSS_PPS_EDGE","REMOTE,GNSS_PPS_EDGE"),encoding="utf-8")
            with self.assertRaisesRegex(ValueError,"mismatched"):
                mod.parse_csv(p)

    def test_fake_physical_manifest_never_passes(self):
        with tempfile.TemporaryDirectory() as td:
            base=pathlib.Path(td)
            p=base/"physical.csv"
            p.write_text(rows("EXTERNAL_ANALYZER"),encoding="utf-8")
            trials,origins,clocks=mod.parse_csv(p)
            incomplete=manifest("HARDWARE_OBSERVATION")
            self.assertTrue(mod.validate_provenance(incomplete,p,trials,origins,clocks))
            data = dict(incomplete)
            data.update({
              "capture_sha256":hashlib.sha256(p.read_bytes()).hexdigest(),
              "instrument":{"model":"TEST_ONLY","asset_id":"SIM","sample_rate_hz":10000000},
              "firmware":{"boat_commit":"a"*40,"remote_commit":"b"*40},
              "pps":{"gnss_module_revision":"SIM","physical_pps_present":True,
                     "pps_utc_second_association_verified":True,"pps_edge_polarity":"RISING",
                     "witness_evidence":"FAKE_TEST_ONLY"},
              "sample_markers":{"boat_gpio_pin":"SIM","remote_gpio_pin":"SIM",
                 "logic_level_verified":True,"time_read_and_marker_latency_calibrated":True,
                 "probe_evidence":"FAKE_TEST_ONLY"},
              "hardware":{"actuator_power_disconnected":True,"radio_rf_audited":True,
                          "remote_board_revision":"SIM","boat_board_revision":"SIM"},
              "reviewer":"UNIT_TEST_ONLY",
            })
            self.assertEqual([],mod.validate_provenance(data,p,trials,origins,clocks))
            m=base/"manifest.json";m.write_text(json.dumps(data),encoding="utf-8")
            self.assertEqual(4,mod.main([str(p),str(m)]))
            p.write_text(rows("EXTERNAL_ANALYZER")+" ",encoding="utf-8")
            self.assertTrue(any("SHA256" in e for e in
                                mod.validate_provenance(data,p,trials,origins,clocks)))

    def test_pps_second_replay_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            p=pathlib.Path(td)/"bad.csv"
            p.write_text(rows().replace(
                edge("T02",2000000,"GNSS","GNSS_PPS_EDGE",EPOCH+1000),
                edge("T02",2000000,"GNSS","GNSS_PPS_EDGE",EPOCH)),encoding="utf-8")
            trial,_,_=mod.parse_csv(p)
            errors,_=mod.compute(trial)
            self.assertTrue(any("reordered/replayed" in e for e in errors))

if __name__=="__main__":
    unittest.main()
