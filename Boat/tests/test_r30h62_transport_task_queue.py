"""R30H-89: source contract for current epoch-tagged STOP handoff.

The R30H-62 contract checked obsolete raw Command queue implementation.
These tests verify observable source invariants; C++ host stress tests
exercise queue semantics and concurrent STOP/rearm separately.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
BOAT = ROOT / "Boat/source/transport_task_queue.h"
REMOTE = ROOT / "Remote/source/transport_task_queue.h"


class TestTransportTaskQueue(unittest.TestCase):
    def test_cross_endpoint_identical_and_stop_priority(self):
        boat = BOAT.read_text()
        remote = REMOTE.read_text()
        self.assertEqual(boat, remote)
        for required in (
            "struct Entry { Command command; uint32_t epoch; };",
            "xQueueCreate(1, sizeof(Entry))",
            "xQueueOverwrite(stop_, &e)",
            "xQueueReceive(stop_, &e, 0)",
            "xQueueReceive(ordinary_, &e, 0)",
            "xQueueReset(ordinary_)",
            "if (!ready()) { end(); return false; }",
            "std::is_trivially_copyable<Command>::value",
            "std::atomic<uint32_t> state_{1U}",
            "std::atomic<uint32_t> acknowledgedStopEpoch_",
        ):
            with self.subTest(required=required):
                self.assertIn(required, boat)
        self.assertNotIn("#include <Arduino.h>", boat)
        self.assertLess(
            boat.index("xQueueReceive(stop_, &e, 0)"),
            boat.index("xQueueReceive(ordinary_, &e, 0)"),
        )

    def test_stop_latches_before_writer_arbitration_and_preserves_pending_stop(self):
        source = BOAT.read_text()
        start = source.index("bool offerStop(")
        end = source.index("bool receive(", start)
        stop = source[start:end]
        self.assertLess(
            stop.index("const uint32_t newEpoch = latchAndAdvance();"),
            stop.index("stopWriter_.test_and_set(std::memory_order_acquire)"),
        )
        self.assertLess(
            stop.index("xQueueOverwrite(stop_, &e)"),
            stop.index("xQueueReset(ordinary_)"),
        )
        self.assertIn("bool clearStopLatchAfterAuthorization()", source)
        self.assertIn("compare_exchange_strong(observed, desired", source)
        self.assertIn("acknowledgedStopEpoch_.load(std::memory_order_acquire)", source)
        self.assertIn("bool mayExecute(uint32_t token) const", source)
        self.assertIn("bool executeIfCurrent(uint32_t token, Fn fn)", source)
        self.assertIn("xSemaphoreCreateMutex()", source)
        self.assertIn("xSemaphoreTake(mutex_, 0)", source)
        self.assertIn("xSemaphoreGive(mutex_)", source)

        # clear() must never discard a STOP payload waiting to be consumed.
        clear_body = source.split("void clear() {", 1)[1].split("bool stopLatched()", 1)[0]
        self.assertIn("latchAndAdvance()", clear_body)
        self.assertIn("xQueueReset(ordinary_)", clear_body)
        self.assertNotIn("xQueueReset(stop_)", clear_body)

    def test_rearm_does_not_remove_or_restore_pending_stop(self):
        source = BOAT.read_text()
        rearm = source.split("bool clearStopLatchAfterAuthorization()", 1)[1].split("void clear()", 1)[0]
        self.assertIn("xQueuePeek(stop_, &pending, 0)", rearm)
        self.assertNotIn("xQueueReceive(stop_", rearm)
        self.assertNotIn("xQueueOverwrite(stop_", rearm)

    def test_nonblocking_stop_and_fail_closed_generation(self):
        source = BOAT.read_text()
        stop = source.split("bool offerStop(", 1)[1].split("bool receive(", 1)[0]
        self.assertNotIn("portMAX_DELAY", source)
        self.assertIn("xQueueOverwrite(stop_, &e)", stop)
        self.assertIn("xSemaphoreTake(mutex_, 0)", stop)
        self.assertIn("const uint32_t terminal=", source)
        self.assertIn("if (stopEpoch >= 0x7FFFFFFEU)", source)
        self.assertIn("token == currentEpoch()", source)


if __name__ == "__main__":
    unittest.main()
