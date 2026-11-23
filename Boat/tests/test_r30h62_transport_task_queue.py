from pathlib import Path
import unittest
ROOT = Path(__file__).resolve().parents[2]
class TestTransportTaskQueue(unittest.TestCase):
 def test_cross_endpoint_identical_and_stop_priority(self):
  boat=(ROOT/"Boat/source/transport_task_queue.h").read_text()
  remote=(ROOT/"Remote/source/transport_task_queue.h").read_text()
  self.assertEqual(boat,remote)
  for required in ("xQueueCreate(1, sizeof(Command))",
                   "xQueueOverwrite(stop_, &cmd)",
                   "xQueueReceive(stop_, &cmd, 0)",
                   "xQueueReceive(ordinary_, &cmd, 0)",
                   "xQueueReset(ordinary_)", "xQueueReset(stop_)",
                   "if (!ready()) { end(); return false; }"):
   self.assertIn(required,boat)
  self.assertIn("std::is_trivially_copyable<Command>::value",boat)
  self.assertIn("if (ready()) return true;",boat)
  self.assertIn("end();",boat)
  self.assertNotIn("#include <Arduino.h>",boat)
  self.assertLess(boat.index("xQueueReceive(stop_, &cmd, 0)"),
                  boat.index("xQueueReceive(ordinary_, &cmd, 0)"))
 def test_stop_invalidates_pending_ordinary_queue(self):
  s=(ROOT/"Boat/source/transport_task_queue.h").read_text()
  start=s.index("bool offerStop(")
  end=s.index("bool receive(",start)
  stop=s[start:end]
  self.assertIn("xQueueOverwrite(stop_, &cmd)",stop)
  self.assertIn("xQueueReset(ordinary_)",stop)
  self.assertLess(stop.index("xQueueOverwrite(stop_, &cmd)"),stop.index("xQueueReset(ordinary_)"))
  self.assertIn("already dequeued",s)
  self.assertIn("stopLatched_.store(true, std::memory_order_release)",stop)
  self.assertIn("clearStopLatchAfterAuthorization()",s)
  self.assertIn("stopLatched_.load(std::memory_order_acquire)",s)
  self.assertIn("xSemaphoreCreateMutex()",s)
  self.assertIn("xSemaphoreTake(mutex_, 0)",s)
  self.assertIn("xSemaphoreGive(mutex_)",s)
  self.assertIn("if (ok) xQueueReset(ordinary_)",stop)
  self.assertIn("caller must retry STOP admission",stop)
 def test_no_blocking_on_stop_path(self):
  s=(ROOT/"Boat/source/transport_task_queue.h").read_text()
  self.assertNotIn("portMAX_DELAY",s)
  self.assertIn("xQueueOverwrite(stop_, &cmd)",s)
if __name__=="__main__":
 unittest.main()
