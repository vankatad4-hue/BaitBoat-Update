"""Source-level regression: authenticated LoRa mission feedback must reach Remote client.

This is not an RF or end-to-end integration test. HMAC, sequence and session
validation happen in validateInboundFrame before dispatchValidatedPayload.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
SRC = (ROOT / "Remote/source/boat_radio.cpp").read_text(encoding="utf-8")
BOAT = (ROOT / "Boat/source/boat_dual_radio_port.cpp").read_text(encoding="utf-8")

class MissionFeedbackBackupContract(unittest.TestCase):
    def test_remote_accepts_both_authenticated_links(self):
        branch = SRC.split("case RADIO_MSG_MISSION_FEEDBACK:", 1)[1].split("case RADIO_MSG_MISSION_ACK:", 1)[0]
        self.assertIn("missionFeedbackDecode(frame.payload,feedback)", branch)
        self.assertIn("g_missionControl.observe(feedback,now,true)", branch)
        self.assertNotIn("link!=LINK_NRF_PRIMARY", branch)

    def test_boat_backup_only_feedback_path(self):
        body = BOAT.split("bool BoatDualRadioPort::sendMissionFeedback(", 1)[1].split("bool BoatDualRadioPort::requestLowBatteryReturnHome(", 1)[0]
        self.assertIn("backupFresh(now)?LINK_LORA_BACKUP:LINK_NONE", body)
        self.assertIn("sendFrame(link,f)", body)

    def test_remote_backup_mission_control_is_discrete(self):
        body = SRC.split("MissionControlRequest request{};", 1)[-1]
        self.assertIn("request.op==MCTRL_PAUSE||request.op==MCTRL_CANCEL", body)
        self.assertIn("g_lora.sendFrame(f,now)", body)

    def test_boat_rejects_backup_manual_mission_ops(self):
        body = BOAT.split("if(f.type==RADIO_MSG_MISSION_CONTROL)", 1)[1].split("if(f.type==RADIO_MSG_MISSION_PREPARE)", 1)[0]
        self.assertIn("link==LINK_LORA_BACKUP&&request.op!=MCTRL_PAUSE&&request.op!=MCTRL_CANCEL", body)

if __name__ == "__main__":
    unittest.main()
