import sys,pathlib,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"tools"))
from power_budget_r7 import LoadInputs,calculate
from safety_gate_spec_r7 import GateLatch,verify
class HardwarePowerTests(unittest.TestCase):
 def test_no_guessed_currents(self):
  with self.assertRaisesRegex(ValueError,"MEASUREMENT"):
   calculate(LoadInputs(9,20,None,None,None,None,None))
 def test_two_exclusive_peak_paths(self):
  # Deliberately synthetic fixture values, NOT measured device currents.
  r=calculate(LoadInputs(9,20,4,4,6,0.8,0.85))
  self.assertGreater(r["motors_peak_a"],r["release_peak_a"])
  self.assertTrue(r["sufficient"])
  r=calculate(LoadInputs(9,20,9,9,6,0.8,0.85))
  self.assertFalse(r["sufficient"])
 def test_watchdog_fault_latch(self):
  verify()
if __name__=="__main__":unittest.main()
