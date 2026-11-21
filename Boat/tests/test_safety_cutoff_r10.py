import itertools
import pathlib
import sys
import unittest
from dataclasses import replace

ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"tools"))
from safety_cutoff_r10 import (SAFE_NAMES, SafetyInputs, CutoffLogicModel, safe_frame)


class HardwareTopologyReferenceTests(unittest.TestCase):
    def test_input_set_matches_r10_schematic_labels(self):
        s=(ROOT/"hardware/Safety_Cutoff_R10/Boat_Safety_Cutoff_R10.kicad_sch").read_text()
        self.assertEqual(len(SAFE_NAMES),10)
        for signal in SAFE_NAMES:
            self.assertIn('(label "'+signal+'"',s)
        self.assertIn('(label "STOP_SOURCE_RETURN"',s)
        self.assertIn('(label "HIGH_SIDE_GATE"',s)
        self.assertIn('(label "DRIVE_ALLOWED"',s)

    def test_1024_input_combinations_fault_closed(self):
        for combo in itertools.product((False,True),repeat=10):
            model=CutoffLogicModel()
            model.step(safe_frame())
            model.step(safe_frame(arm=True))
            x=SafetyInputs(tuple(combo),True,True,False)
            self.assertEqual(model.step(x),all(combo),combo)
            if not all(combo):
                # ARM held while the fault clears must not create a clean edge.
                self.assertFalse(model.step(safe_frame(arm=True,permit=False)))
                self.assertFalse(model.step(safe_frame(arm=True)))
                self.assertFalse(model.step(safe_frame()))
                self.assertTrue(model.step(safe_frame(arm=True)))

    def test_stop_contact_breaks_gate_even_when_stop_feedback_stuck_good(self):
        m=CutoffLogicModel()
        m.step(safe_frame())
        self.assertTrue(m.step(safe_frame(arm=True)))
        self.assertFalse(m.step(safe_frame(arm=True,stop=False)))
        self.assertFalse(m.step(safe_frame(arm=True,stop=False,permit=False)))
        # Fail closed for *current* output when physically cut; hardware Q1
        # short-failure is NOT covered by this model.
        # Independent S2 opened while S1 interrupted Q2 return, so
        # releasing STOP cannot reenergize without a NEW arm sequence.
        self.assertFalse(m.step(safe_frame(arm=True,stop=True)))
        self.assertFalse(m.step(safe_frame()))
        self.assertTrue(m.step(safe_frame(arm=True)))

    def test_one_monitor_stuck_good_still_latches_on_stop(self):
        for second_pole_stuck, witness_stuck in ((True,False),(False,True)):
            m=CutoffLogicModel()
            m.step(safe_frame())
            self.assertTrue(m.step(safe_frame(arm=True)))
            statuses=(True,)*10 if second_pole_stuck else (False,)+(True,)*9
            failed=SafetyInputs(statuses,False,True,True,witness_stuck)
            self.assertFalse(m.step(failed))
            self.assertFalse(m.step(safe_frame(arm=True)))
            self.assertFalse(m.step(safe_frame()))
            self.assertTrue(m.step(safe_frame(arm=True)))

    def test_stuck_arm_and_power_up_and_permit(self):
        m=CutoffLogicModel()
        self.assertFalse(m.step(safe_frame(arm=True)))
        self.assertFalse(m.step(safe_frame(arm=True)))
        self.assertFalse(m.step(safe_frame()))
        self.assertTrue(m.step(safe_frame(arm=True)))
        self.assertFalse(m.step(safe_frame(arm=True,permit=False)))
        self.assertTrue(m.step(safe_frame(arm=True,permit=True)))
        bad=replace(safe_frame(),safe=(False,)+(True,)*9)
        self.assertFalse(m.step(bad))
        self.assertFalse(m.step(safe_frame(arm=True)))

    def test_invalid_inputs_denied(self):
        m=CutoffLogicModel()
        self.assertFalse(m.step(SafetyInputs((True,),True,True,False)))
        self.assertFalse(m.step(SafetyInputs(tuple([True]*9+[None]),True,True,False)))


if __name__=="__main__": unittest.main()
