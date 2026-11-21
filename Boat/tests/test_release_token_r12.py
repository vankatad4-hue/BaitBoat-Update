import pathlib
import sys
import unittest
from dataclasses import replace

HERE=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(HERE/"tools"))
from consumed_release_token_r12 import ConsumedReleaseToken,HardwareInputs


class ConsumedTokenTemporalTests(unittest.TestCase):
    def good(self):return HardwareInputs()

    def prime(self,engine):
        # timing values are SYNTHETIC NOT manufacturer coil pulse specs
        self.assertFalse(engine.step(0,self.good()))
        self.assertFalse(engine.step(5,self.good()))
        self.assertFalse(engine.step(54,self.good()))
        self.assertFalse(engine.step(56,self.good()))
        self.assertTrue(engine.armed)

    def test_held_high_never_retriggers_even_after_main_window(self):
        t=ConsumedReleaseToken()
        self.prime(t)
        hold=replace(self.good(),request_mask=1)
        self.assertFalse(t.step(100,hold))
        self.assertTrue(t.step(106,hold))
        self.assertTrue(t.step(110,hold))
        self.assertFalse(t.step(115,hold))
        for now in (120,160,280,500,5000,10000):
            self.assertFalse(t.step(now,hold))
            self.assertFalse(t.armed)
        self.assertTrue(t.fired)

    def test_each_channel_after_qualified_release(self):
        t=ConsumedReleaseToken()
        now=0
        for bit in (1,2,4,8):
            t.step(now,self.good())
            t.step(now+5,self.good())
            t.step(now+60,self.good())
            self.assertTrue(t.armed)
            hold=replace(self.good(),request_mask=bit)
            self.assertFalse(t.step(now+70,hold))
            self.assertTrue(t.step(now+76,hold))
            self.assertFalse(t.armed)
            self.assertFalse(t.step(now+86,hold))
            # allow stable low so next iteration can rearm
            t.step(now+100,self.good())
            t.step(now+110,self.good())
            now+=200

    def test_short_release_not_qualified(self):
        t=ConsumedReleaseToken()
        self.prime(t)
        press=replace(self.good(),request_mask=1)
        t.step(100,press);self.assertTrue(t.step(105,press))
        t.step(160,self.good());t.step(166,self.good())
        self.assertFalse(t.armed)
        t.step(170,press)
        self.assertFalse(t.step(177,press))
        self.assertFalse(t.step(220,press))
        self.assertFalse(t.armed)

    def test_overlap_and_invalid_channels_all_rejected(self):
        for mask in (0,3,5,6,7,9,10,11,12,13,14,15):
            t=ConsumedReleaseToken()
            self.prime(t)
            bad=replace(self.good(),request_mask=mask)
            t.step(100,bad)
            self.assertFalse(t.step(105,bad),mask)
            self.assertFalse(t.step(110,bad),mask)

    def test_por_brownout_latch_and_current_inputs(self):
        for flag in ("latch_ok","por_ok","rail_ok"):
            t=ConsumedReleaseToken()
            self.prime(t)
            t.step(100,replace(self.good(),**{flag:False}))
            self.assertFalse(t.armed)
            self.assertFalse(t.step(110,replace(self.good(),request_mask=1)))
            self.assertFalse(t.step(120,replace(self.good(),request_mask=1)))
        for flag in ("motor_current_zero","coil_current_zero"):
            t=ConsumedReleaseToken()
            t.step(0,self.good())
            t.step(5,self.good())
            t.step(56,replace(self.good(),**{flag:False}))
            self.assertFalse(t.armed)

    def test_negative_and_backwards_clock_reset(self):
        t=ConsumedReleaseToken()
        self.prime(t)
        self.assertFalse(t.step(55,self.good()))
        self.assertFalse(t.armed)
        self.assertFalse(t.step(-1,self.good()))
        self.assertFalse(t.armed)

    def test_glitch_shorter_than_filter_does_not_count_as_release(self):
        t=ConsumedReleaseToken()
        self.prime(t)
        high=replace(self.good(),request_mask=1)
        t.step(100,high);self.assertTrue(t.step(106,high))
        # momentary 2ms low cannot clear the qualified request.
        t.step(120,self.good())
        self.assertFalse(t.step(122,high))
        self.assertFalse(t.step(140,high))
        self.assertFalse(t.armed)


if __name__=="__main__":
    unittest.main()
