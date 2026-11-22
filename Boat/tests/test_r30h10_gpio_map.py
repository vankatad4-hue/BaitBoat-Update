import importlib.util
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[2]
sp=importlib.util.spec_from_file_location("r30h_gpio",ROOT/"Boat/tools/verify_r30h10_gpio_map.py")
mod=importlib.util.module_from_spec(sp)
sp.loader.exec_module(mod)
class TestMap(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.source=mod.snapshot(ROOT)
    def test_unsigned_plan_passes(self):self.assertEqual([],mod.verify(self.source))
    def flip(self,path,old,new):
        d=dict(self.source);self.assertIn(old,d[path]);d[path]=d[path].replace(old,new,1)
        self.assertTrue(mod.verify(d))
    def test_ce_must_remain_unassigned(self):
        self.flip(mod.RH,"#define REMOTE_NRF_CE_PIN   (-1)","#define REMOTE_NRF_CE_PIN   6")
    def test_csn_must_remain_unassigned(self):
        self.flip(mod.RH,"#define REMOTE_NRF_CSN_PIN  (-1)","#define REMOTE_NRF_CSN_PIN  15")
    def test_pwm_stays_locked(self):
        self.flip(mod.B,"MDD_PWM1,39,J3-9,BLOCKED_ACTUATOR","MDD_PWM1,39,J3-9,PROBE_ONLY")
    def test_pps_not_invented(self):
        self.flip(mod.B,"GNSS_PPS,-1,UNASSIGNED,UNASSIGNED","GNSS_PPS,2,J3-5,PROBE_ONLY")
