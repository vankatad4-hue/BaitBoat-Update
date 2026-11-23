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

    def test_r30h17_stop_ack_correlation_cannot_disappear(self):
        self.check_rejects_edit("Remote/source/boat_radio.cpp",
             "g_stopConfirmation.allowsStopStatus(true,decoded.stopState==1)",
             "decoded.stopState==1")

    def test_stop_confirmation_gate_cannot_disappear(self):
        self.check_rejects_edit("Remote/source/boat_radio.cpp",
             "g_primaryStopRetry.noteAdmitted(millis());",
             "g_primaryStopRetry.reset();")

    def test_stop_priority_order_cannot_move_behind_heartbeat(self):
        src=(ROOT/"Remote/source/boat_radio.cpp").read_text(encoding="utf-8")
        a="g_primaryStopRetry.mayAttempt(sessionReady(),g_nrfReady,"
        b="g_primaryStopRetry.maySendPeriodic(g_nrfReady,g_nrf.txPending())"
        self.assertIn(a,src)
        self.assertIn(b,src)
        modified=src.replace(a,"NONSTOP_PLACEHOLDER",1).replace(b,a,1)
        modified=modified.replace("NONSTOP_PLACEHOLDER",b,1)
        errors,_=audit_module.audit(overrides={
            "Remote/source/boat_radio.cpp":modified})
        self.assertTrue(any("STOP priority broken" in e for e in errors),errors)

    def test_boat_stop_ack_priority_before_heartbeat(self):
        src=(ROOT/"Boat/source/boat_dual_radio_port.cpp").read_text(encoding="utf-8")
        a="stopAck_.due(ackNow,sessionReady(),nrfReady_,nrf_.txPending())"
        b="sendHeartbeat(LINK_NRF_PRIMARY,now)"
        self.assertIn(a,src)
        self.assertIn(b,src)
        modified=src.replace(a,"BOAT_STOP_ACK_PLACEHOLDER",1).replace(b,a,1)
        modified=modified.replace("BOAT_STOP_ACK_PLACEHOLDER",b,1)
        errors,_=audit_module.audit(overrides={"Boat/source/boat_dual_radio_port.cpp":modified})
        self.assertTrue(any("Boat STOP ACK/STATUS priority broken" in e for e in errors),errors)

    def test_boat_stop_ack_cannot_be_silently_dropped(self):
        self.check_rejects_edit("Boat/source/boat_dual_radio_port.cpp",
            "if(stopAck_.pending()||statusOutbox_.pending())return false;",
            "if(false)return false;")

    def test_boat_stop_status_priority_before_heartbeat(self):
        src=(ROOT/"Boat/source/boat_dual_radio_port.cpp").read_text(encoding="utf-8")
        a="statusOutbox_.due(statusNow,sessionReady(),nrfReady_,"
        b="sendHeartbeat(LINK_NRF_PRIMARY,now)"
        self.assertIn(a,src)
        self.assertIn(b,src)
        modified=src.replace(a,"BOAT_STATUS_PLACEHOLDER",1).replace(b,a,1)
        modified=modified.replace("BOAT_STATUS_PLACEHOLDER",b,1)
        errors,_=audit_module.audit(overrides={"Boat/source/boat_dual_radio_port.cpp":modified})
        self.assertTrue(any("Boat STOP ACK/STATUS priority broken" in e for e in errors),errors)

    def test_r30h21_heartbeat_sequence_budget_guard_must_precede_frame_build(self):
        src=(ROOT/"Boat/source/boat_dual_radio_port.cpp").read_text(encoding="utf-8")
        start=src.find("bool BoatDualRadioPort::sendHeartbeat(")
        end=src.find("bool BoatDualRadioPort::primaryFresh(",start)
        self.assertGreaterEqual(start,0)
        self.assertGreater(end,start)
        segment=src[start:end]
        guard="if(link==LINK_NRF_PRIMARY&&!primaryOrdinaryTxReady())return false;"
        self.assertIn(guard,segment)
        modified=src[:start]+segment.replace(guard,"if(false)return false;",1)+src[end:]
        errors,_=audit_module.audit(overrides={"Boat/source/boat_dual_radio_port.cpp":modified})
        self.assertTrue(any("R30H-21 busy-TX heartbeat" in e for e in errors),errors)

    def test_r30h21_pending_status_and_stop_ack_deny_ordinary_tx(self):
        self.check_rejects_edit("Boat/source/boat_r30h21_primary_tx_budget.h",
            "!s.stopAckPending && !s.statusPending;",
            "true;")

    def test_remote_radio_pin_lock_cannot_silently_disappear(self):
        self.check_rejects_edit("Remote/source/radio_hw_config.h",
             "#define REMOTE_NRF_PIN_AUDIT_SIGNED 0",
             "#define REMOTE_NRF_PIN_AUDIT_SIGNED 1")
