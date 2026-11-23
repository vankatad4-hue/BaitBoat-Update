"""Portable source-contract checks for busy E22 mission reply retry."""
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[2]
SRC=(ROOT/"Boat/source/boat_dual_radio_port.cpp").read_text(encoding="utf-8")
HDR=(ROOT/"Boat/source/boat_dual_radio_port.h").read_text(encoding="utf-8")
class FeedbackRetry(unittest.TestCase):
    def test_one_pending_correlated_reply(self):
        self.assertIn("MissionControlFeedback pendingBackupMissionFeedback_",HDR)
        self.assertIn("pendingBackupMissionFeedback_=status",SRC)
        self.assertIn("status.result!=MC_STATUS",SRC)
    def test_distinct_request_does_not_overwrite_queued_reply(self):
        self.assertIn('if(pendingBackupMissionFeedbackValid_){',SRC)
        self.assertIn('Duplicate requests must not refresh expiry',SRC)
        self.assertIn('pendingBackupMissionFeedbackAtMs_=now',SRC)
    def test_retry_before_periodic_status_and_heartbeat(self):
        self.assertLess(SRC.index("if(pendingBackupMissionFeedbackValid_){",SRC.index("void BoatDualRadioPort::update")),
                        SRC.index("const auto progress=controller_.status()"))
        self.assertIn("lastBackupMissionFeedbackAttemptMs_=retryNow",SRC)
    def test_session_reset_and_expiry(self):
        self.assertIn("pendingBackupMissionFeedbackValid_=false;lastBackupMissionFeedbackAttemptMs_=0",SRC)
        self.assertIn("(retryNow-pendingBackupMissionFeedbackAtMs_)>12000U",SRC)
    def test_no_status_overwrites_correlated_reply(self):
        self.assertIn("pendingBackupMissionFeedbackValid_&&status.result==MC_STATUS",SRC)
if __name__=="__main__":
    unittest.main()
