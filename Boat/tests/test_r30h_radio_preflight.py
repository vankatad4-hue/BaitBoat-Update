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

    def test_fail_if_canonical_e22_profile_changes(self):
        self.hostile("Common/source/e22_uart_link.cpp",
                     "0x67,0xC3,0x12", "0x67,0xC3,0x13")

    def test_fail_if_staged_command_codec_drifts(self):
        self.hostile("Remote/source/radio_command_codec.cpp",
                     "if (cmd.type == CMD_EMERGENCY_STOP) return true;",
                     "if (cmd.type == CMD_EMERGENCY_STOP) return false;")

    def test_fail_if_r30h3_phys_latency_fabricated(self):
        self.hostile("Boat/docs/BOAT_R30H_3_STOP_RF_ACCEPTANCE.csv",
                     "NOT_RUN,,,,", "PASS,100,200,unverified,")

    def test_fail_if_fake_physical_signoff(self):
        self.hostile("Boat/docs/BOAT_R30H_1_RF_MODULE_SIGNOFF.csv",
                     "NOT_RUN,,,", "PASS,dummy,not_verified,")

    def test_r30h4_fake_manifest_cannot_be_approved(self):
        self.hostile("Boat/docs/BOAT_R30H_4_CAPTURE_MANIFEST_TEMPLATE.json",
                     '"evidence_mode": "UNSPECIFIED"',
                     '"evidence_mode": "HARDWARE_OBSERVATION"')

    def test_r30h4_probe_cannot_be_signed(self):
        self.hostile("Boat/docs/BOAT_R30H_4_RF_ONLY_PROBE_CROSSWALK.csv",
                     "NOT_READY,,,,,", "PASS,not_verified,3.3,0.0,none,none")

    def test_r30h4_authoritative_raw_capture_cannot_be_fabricated(self):
        self.hostile("Boat/docs/BOAT_R30H_4_LOGIC_CAPTURE_TEMPLATE.csv",
                     "time_us,platform,event,edge,origin,notes\n",
                     "time_us,platform,event,edge,origin,notes\nC01,T01,H05,FAKE,100,REMOTE,STOP_BUTTON_EDGE,RISING,EXTERNAL_ANALYZER,dummy\n")

    def test_r30h7_mismatched_gnss_epoch_does_not_qualify_utc(self):
        self.hostile("Boat/source/tracker_gnss.h",
                     "positionUtc_!=rmcUtc_", "false /* fake GNSS epoch */")

    def test_r30h7_remote_utc_needs_primary_freshness(self):
        self.hostile("Remote/source/boat_radio.cpp",
                     "g_linkManager.primaryHealthy(at)&&",
                     "true&&")

    def test_r30h7_different_utc_wire_header_rejected(self):
        self.hostile("Remote/source/radio_utc_time_v1.h",
                     "static constexpr uint8_t VERSION=1;",
                     "static constexpr uint8_t VERSION=2;")

    def test_r30h7_no_fabricated_rf_reference(self):
        self.hostile("Boat/source/utc_marker_policy.h",
                     "bool rfEndToEndStopClock=false;",
                     "bool rfEndToEndStopClock=true;")

    def test_r30h7_fake_pps_signoff_rejected(self):
        self.hostile("Boat/docs/BOAT_R30H_7_GNSS_PPS_RF_CLOCK_SIGNOFF.csv",
                     "NOT_RUN,,,", "PASS,PPS=real,unverified,none")

    def test_r30h8_fake_pps_data_is_denied(self):
        self.hostile("Boat/docs/BOAT_R30H_8_SHARED_CLOCK_CAPTURE_TEMPLATE.csv",
                     "event,utc_unix_ms,origin,notes\n",
                     "event,utc_unix_ms,origin,notes\nC1,T1,FAKE,0,GNSS,GNSS_PPS_EDGE,1795347296000,EXTERNAL_ANALYZER,dummy\n")

    def test_r30h8_manifests_cannot_claim_physical_pps(self):
        self.hostile("Boat/docs/BOAT_R30H_8_SHARED_CLOCK_MANIFEST_TEMPLATE.json",
                     '"evidence_mode": "UNSPECIFIED"',
                     '"evidence_mode": "HARDWARE_OBSERVATION"')

    def test_r30h8_clock_gate_cannot_be_signed(self):
        self.hostile("Boat/docs/BOAT_R30H_8_CLOCK_ALIGNMENT_GATES.csv",
                     "NOT_RUN,,,", "PASS,test_only,unverified,none")

    def test_fail_if_motor_driver_attaches(self):
        self.hostile("Boat/source/firmware/Flytec_Boat.ino",
                     "const bool ready=radio.begin();",
                     "application.attach(fakeDriver,fakeConfig);\n const bool ready=radio.begin();")

if __name__ == "__main__":
    unittest.main()
