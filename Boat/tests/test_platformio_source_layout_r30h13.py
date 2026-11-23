import importlib.util
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location(
    "pio_layout",ROOT/"Boat/tools/verify_platformio_source_layout_r30h13.py")
audit_module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit_module)

class TestSourceLayout(unittest.TestCase):
    def test_all_active_boat_remote_sources_and_shares(self):
        errors,report=audit_module.audit()
        self.assertEqual([],errors)
        self.assertGreaterEqual(report["endpoint_source_files"]["Boat"],95)
        self.assertGreaterEqual(report["endpoint_source_files"]["Remote"],95)
        self.assertEqual(0,report["deprecated_remote_direct_nrf_sources"])
        self.assertEqual("PASS_SOURCE_ONLY",report["verdict"])

    def check_rejects_edit(self,path,old,new):
        self.assertIn(old,(ROOT/path).read_text(encoding="utf-8"))
        edited=(ROOT/path).read_text(encoding="utf-8").replace(old,new,1)
        errors,_=audit_module.audit(overrides={path:edited})
        self.assertTrue(errors)

    def test_direct_remote_rf24_dependency_cannot_return(self):
        self.check_rejects_edit("Remote/platformio.ini",
             "lib_ldf_mode = deep",
             "lib_ldf_mode = deep\n  nrf24/RF24 @ 1.4.11")

    def test_shared_authenticated_protocol_must_remain_identical(self):
        self.check_rejects_edit("Remote/source/radio_protocol.h",
             "RADIO_FRAME_SIZE = 32", "RADIO_FRAME_SIZE = 31")

    def test_boat_tx_watchdog_cannot_silently_disappear(self):
        self.check_rejects_edit("Boat/source/nrf24_link.h",
             "TX_PENDING_TIMEOUT_MS = 120U","TX_PENDING_TIMEOUT_MS = 0U")

    def test_cross_endpoint_radio_channel_mismatch_is_rejected(self):
        self.check_rejects_edit("Remote/source/radio_hw_config.h",
             "#define REMOTE_NRF_CHANNEL  76",
             "#define REMOTE_NRF_CHANNEL  77")

    def test_boat_retry_mismatch_is_rejected(self):
        self.check_rejects_edit("Boat/source/nrf24_link.cpp",
             "radio_.setRetries(5,15);","radio_.setRetries(5,10);")

    def test_remote_radio_pin_lock_cannot_silently_disappear(self):
        self.check_rejects_edit("Remote/source/radio_hw_config.h",
             "#define REMOTE_NRF_PIN_AUDIT_SIGNED 0",
             "#define REMOTE_NRF_PIN_AUDIT_SIGNED 1")
