"""Static regression of E22 mission retry and scheduler ordering."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
RADIO=(ROOT/"Remote/source/boat_radio.cpp").read_text(encoding="utf-8")
CLIENT=(ROOT/"Remote/source/mission_control_client.h").read_text(encoding="utf-8")

class BackupMissionPriority(unittest.TestCase):
    def test_backup_timeout_exceeds_transmit_interval(self):
        self.assertIn("retryMs=backup?3000U:400U",CLIENT)
        self.assertIn("timeoutMs=backup?12000U:1800U",CLIENT)

    def test_discrete_mission_sent_before_backup_heartbeat(self):
        self.assertLess(RADIO.index("g_missionControl.next(now,request,missionBackup)"),
                        RADIO.index("if(sessionReady()&&g_loraConfigured&&(!g_backupTxSent"))

    def test_pending_stop_is_serviced_before_mission(self):
        self.assertLess(RADIO.index("if(g_stopPending){",RADIO.index("void boatRadioUpdate")),
                        RADIO.index("g_missionControl.next(now,request,missionBackup)"))

    def test_backup_mission_retry_requires_available_slot(self):
        self.assertIn("(!missionBackup||!g_backupTxSent||(uint32_t)(now-g_lastBackupTxMs)>=3000U)",RADIO)

if __name__=="__main__":
    unittest.main()
