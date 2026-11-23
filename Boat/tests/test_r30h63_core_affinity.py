from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[2]
class TestCoreAffinity(unittest.TestCase):
 def test_both_targets_observe_setup_and_loop(self):
  for target,path in (("Boat","Boat/source/firmware/Flytec_Boat.ino"),("Remote","Remote/source/Flytec_Remote.ino")):
   with self.subTest(target=target):
    source=(ROOT/path).read_text()
    self.assertIn("xPortGetCoreID()",source)
    self.assertIn("portNUM_PROCESSORS",source)
    self.assertIn("R30H-63 "+target+" setup core=",source)
    self.assertIn("R30H-63 "+target+" loop core=",source)
    self.assertIn("r30h63CoreReported",source)
if __name__=="__main__":unittest.main()
