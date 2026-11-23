#!/usr/bin/env python3
"""R30H-71 source-contract guard. Not a substitute for FreeRTOS concurrency tests."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
class TestHandoffContract(unittest.TestCase):
    def test_identical_implementations(self):
        boat = (ROOT / "Boat/source/transport_task_queue.h").read_text()
        remote = (ROOT / "Remote/source/transport_task_queue.h").read_text()
        self.assertEqual(boat, remote)
    def test_stop_and_epoch_invariants(self):
        src = (ROOT / "Boat/source/transport_task_queue.h").read_text()
        self.assertIn("std::atomic<uint32_t> state_{1U}", src)
        self.assertIn("std::atomic<uint32_t> acknowledgedStopEpoch_", src)
        self.assertIn("xQueueOverwrite(stop_, &e)", src)
        self.assertIn("compare_exchange_strong(observed, desired", src)
        self.assertIn("bool receive(Command& cmd, bool& isStop, uint32_t& epochToken)", src)
        self.assertIn("bool mayExecute(uint32_t token) const", src)
        self.assertIn("bool executeIfCurrent(uint32_t token, Fn fn)", src)
        self.assertIn("return gotStop;", src)
        self.assertNotIn("xQueueReset(ordinary_); xQueueReset(stop_);", src)
        self.assertIn("clear() is a fail-closed cancellation", src)
    def test_no_radio_worker_activation(self):
        src = (ROOT / "Boat/source/transport_task_queue.h").read_text()
        self.assertNotIn("xTaskCreatePinnedToCore", src)
if __name__ == "__main__":
    unittest.main()
