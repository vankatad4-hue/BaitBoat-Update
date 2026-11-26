from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class TestWorkerAirtimeCommissioningContract(unittest.TestCase):
    def test_e22_admission_stays_closed_until_quiet_profile_is_commissioned(self):
        source = (ROOT / "Boat/source/boat_radio_physical_worker.h").read_text()
        start = source.index("if(e22Initialized_&&!e22AirtimeConfigured_)")
        end = source.index("if(sharedNrf_&&sharedE22_)", start)
        gate = source[start:end]
        for term in (
            "e22_.configured()&&!e22_.txPending()&&!e22_.transportFault()",
            "e22QuietRestart_.observe(now,quiet)",
            "quiet&&e22AirtimeEvidenceSigned_&&e22Profile_.commissioned",
            "e22TailMs_>0U&&e22TailMs_<60000U",
            "e22QuietRestart_.ready(now)",
            "e22Airtime_.configureAfterQuiet(e22Profile_,now,e22QuietRestart_)",
        ):
            self.assertIn(term, gate)
        self.assertEqual(source.count("e22Airtime_.configureAfterQuiet("), 1)
        self.assertIn("enabled&&e22Initialized_&&e22AirtimeConfigured_&&e22_.ready()", source)

    def test_compile_only_config_explicitly_keeps_airtime_uncommissioned(self):
        for relative in (
            "Boat/source/boat_dual_radio_port.h",
            "Boat/source/boat_radio_physical_compile.cpp",
        ):
            source = (ROOT / relative).read_text()
            self.assertIn("flytec_rtos::LoRaAirtimeProfile{},0U,false", source)

    def test_managed_e22_binding_blocks_direct_writes_without_double_charging(self):
        policy = (ROOT / "Common/source/lora_link_policy.h").read_text()
        driver = (ROOT / "Common/source/e22_uart_link.cpp").read_text()
        header = (ROOT / "Common/source/e22_uart_link.h").read_text()
        board = (ROOT / "Boat/source/e22_uart_board_port.h").read_text()
        self.assertIn("if(!approved_||binding_==Binding::Rejected||", policy)
        self.assertIn("binding_==Binding::Bound&&!policy_.managedReady()", policy)
        self.assertIn("trySendFrameManaged(f,now)", policy)
        self.assertIn("airtime_.hasCharges()", driver)
        self.assertIn("if(managedAdmissionBound_!=managed)return Admission::Unavailable", driver)
        self.assertIn("if(!managed){", driver)
        self.assertIn("||managedTxActive_)return false", driver)
        self.assertIn("finishManagedAdmission();", policy)
        self.assertIn("finishManagedAdmission()", driver)
        self.assertIn("trySendFrameManaged(const RadioFrame &frame,uint32_t nowMs)", header)
        self.assertIn("trySendFrameManaged(const RadioFrame& f,uint32_t ms)", board)


if __name__ == "__main__":
    unittest.main()
