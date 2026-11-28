#!/usr/bin/env python3
"""Host hostile-mutation tests; never a physical GNSS/PPS/radio measurement."""
import importlib.util
import pathlib
import unittest

ROOT=pathlib.Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location(
    "utc_isolation",ROOT/"Boat/tools/verify_r30h5_gps_time_isolation.py")
audit=importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)

class TestGNSSUTCIsolation(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.sources=audit.snapshot()
    def test_current_source_locked_and_isolated(self):
        self.assertEqual([],audit.verify(self.sources))
    def hostile(self,path,old,new):
        text=dict(self.sources)
        self.assertIn(old,text[path])
        text[path]=text[path].replace(old,new,1)
        self.assertTrue(audit.verify(text),f"accepted unsafe edit {path}")
    def test_rtc_not_allowed_in_safety_controller(self):
        self.hostile("Boat/source/boat_radio_safety.h",
                     "class BoatRadioWatchdog", "void settimeofday();\nclass BoatRadioWatchdog")
    def test_freshness_must_not_change(self):
        self.hostile("Boat/source/boat_dual_radio_port.cpp",
                     "(uint32_t)(now-lastBackupRxMs_)<=6000",
                     "(uint32_t)(now-lastBackupRxMs_)<=60000")
    def test_time_beacon_must_not_refresh_command_link(self):
        self.hostile("Remote/source/boat_radio.cpp",
                     "frame.payload[0]==flytec_time::SUBTYPE))return;",
                     "frame.payload[0]==flytec_time::SUBTYPE)){}")
    def test_dispatch_cannot_refresh_delayed_payload(self):
        self.hostile("Remote/source/boat_radio.cpp",
                     "if(!dispatchValidatedPayload(link,frame,capturedAt))return;",
                     "if(!dispatchValidatedPayload(link,frame,now))return;")
    def test_delayed_frame_cannot_refresh_link(self):
        self.hostile("Remote/source/boat_radio.cpp",
                     "g_linkManager.reportValidRx(link,true,capturedAt);",
                     "g_linkManager.reportValidRx(link,true,now);")
    def test_remote_cannot_skip_hmac(self):
        self.hostile("Remote/source/boat_radio.cpp",
                     "frame.sessionEpoch!=g_sessionEpoch||!radio_frameVerify(frame,g_authKey)",
                     "frame.sessionEpoch!=g_sessionEpoch")
    def test_time_beacon_cannot_preempt_mission(self):
        self.hostile("Boat/source/boat_dual_radio_port.cpp",
                     "!opportunity.commandQueued&&!opportunity.activeMission&&\n    !opportunity.failsafeOrLinkLoss",
                     "!opportunity.commandQueued&&true&&\n    !opportunity.failsafeOrLinkLoss")
    def test_primary_pending_wrapper_must_report_driver(self):
        self.hostile("Boat/source/boat_dual_radio_port.cpp",
                     "bool BoatDualRadioPort::primaryDriverPending() const{return nrf_.txPending();}",
                     "bool BoatDualRadioPort::primaryDriverPending() const{return false;}")
    def test_remote_rf_audit_remains_unsigned(self):
        self.hostile("Remote/source/radio_hw_config.h",
                     "#define REMOTE_NRF_PIN_AUDIT_SIGNED 0",
                     "#define REMOTE_NRF_PIN_AUDIT_SIGNED 1")
    def test_shared_time_protocol_must_match(self):
        self.hostile("Remote/source/radio_utc_time_v1.h",
                     "SUBTYPE=9","SUBTYPE=10")

if __name__=="__main__":
    unittest.main()
