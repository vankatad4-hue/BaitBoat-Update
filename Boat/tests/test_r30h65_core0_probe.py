from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[2]
class TestCoreProbe(unittest.TestCase):
 def test_opt_in_only_and_no_transport_access(self):
  a=(ROOT/"Boat/source/core0_probe.h").read_text()
  b=(ROOT/"Remote/source/core0_probe.h").read_text()
  self.assertEqual(a,b)
  for term in ("#define FLYTEC_ENABLE_CORE0_PROBE 0",
               "#if FLYTEC_ENABLE_CORE0_PROBE",
               "xTaskCreatePinnedToCore(", "vTaskDelete(nullptr)",
               "nullptr,0)==pdPASS", "static_assert(portNUM_PROCESSORS >= 2"):
   self.assertIn(term,a)
  for term in ("boatRadio_", "radio.begin(", "lv_timer_handler", "Wire.", "SPI."):
   self.assertNotIn(term,a)
 def test_both_setups_include_opt_in_probe(self):
  for path in ("Boat/source/firmware/Flytec_Boat.ino","Remote/source/Flytec_Remote.ino"):
   s=(ROOT/path).read_text()
   self.assertIn('#include "core0_probe.h"',s)
   self.assertIn("FLYTEC_ENABLE_CORE0_PROBE && !flytec_core_probe::start()",s)
if __name__=="__main__": unittest.main()
