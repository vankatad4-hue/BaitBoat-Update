#!/usr/bin/env python3
"""Host negative tests; hardware never commissioned or probed."""
import copy
import importlib.util
import pathlib
import unittest

BASE = pathlib.Path(__file__).resolve().parents[2]
SPEC=importlib.util.spec_from_file_location(
    "r30h9_preflight",BASE/"Boat/tools/verify_r30h9_marker_preflight.py")
mod=importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(mod)

class MarkerGateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base=mod.snapshot()
        errs=mod.verify(cls.base)
        if errs:
            raise AssertionError("Unsigned source preflight unexpectedly failed: "+repr(errs))

    def mutation(self,path,original,replacement):
        candidate=copy.deepcopy(self.base)
        self.assertIn(original,candidate[path])
        candidate[path]=candidate[path].replace(original,replacement,1)
        errors=mod.verify(candidate)
        self.assertTrue(errors,f"R30H-9 mutation of {path} accepted: {original}")
        return errors

    def test_baseline_does_not_approve_gpio(self):
        self.assertEqual([],mod.verify(self.base))
        self.assertIn("return false;",self.base["Boat/source/diagnostic_marker_contract.h"])

    def test_boat_numeric_pin_blocks(self):
        self.mutation("Boat/source/boat_diagnostic_marker_config.h",
            "#define BOAT_R30H9_MARKER_GPIO (-1)","#define BOAT_R30H9_MARKER_GPIO 2")

    def test_remote_unsigned_pin_blocks(self):
        self.mutation("Remote/source/remote_diagnostic_marker_config.h",
            "#define REMOTE_R30H9_MARKER_GPIO (-1)","#define REMOTE_R30H9_MARKER_GPIO 15")
        self.mutation("Remote/source/remote_diagnostic_marker_config.h",
            "#define REMOTE_R30H9_GPIO_OUTPUT_SIGNED 0","#define REMOTE_R30H9_GPIO_OUTPUT_SIGNED 1")

    def test_boat_output_signoff_cannot_be_automated(self):
        self.mutation("Boat/source/boat_diagnostic_marker_config.h",
            "#define BOAT_R30H9_GPIO_OUTPUT_SIGNED 0","#define BOAT_R30H9_GPIO_OUTPUT_SIGNED 1")

    def test_gpio_driver_and_mirrored_policy_blocked(self):
        self.mutation("Boat/source/diagnostic_marker_contract.h",
            "return false;","return true;")
        self.mutation("Remote/source/diagnostic_marker_contract.h",
            "SOFTWARE_LOCAL_UNCERTIFIED","PHYSICAL_AUTOMATICALLY_CERTIFIED")
        self.mutation("Boat/source/boat_diagnostic_marker_config.h",
            "// No spare diagnostic GPIO","pinMode(2,OUTPUT); // No spare diagnostic GPIO")

    def test_live_source_must_include_lock(self):
        self.mutation("Boat/source/boat_dual_radio_port.h",
            '#include "boat_diagnostic_marker_config.h"', "// unlawfully removed config")
        self.mutation("Remote/source/boat_radio.cpp",
            "REMOTE_R30H9_GPIO_OUTPUT_SIGNED == 0",
            "REMOTE_R30H9_GPIO_OUTPUT_SIGNED == 1")

    def test_fake_electrical_evidence_denied(self):
        self.mutation("Boat/docs/BOAT_R30H_9_PHYSICAL_SIGNOFF.csv",
            "NOT_RUN,,,","PASS,signal good,trace.png,yes")
        self.mutation("Boat/docs/BOAT_R30H_9_PROBE_CROSSWALK.csv",
            "NOT_RUN,","PASS,")
        
    def test_active_marker_pin_assigned_without_review_denied(self):
        self.mutation("Boat/docs/BOAT_R30H_9_PROBE_CROSSWALK.csv",
            "UNASSIGNED,ACTIVE_DIAGNOSTIC_OUTPUT_NOT_IMPLEMENTED",
            "1,ACTIVE_DIAGNOSTIC_OUTPUT_NOT_IMPLEMENTED")

    def test_remote_existing_signal_must_remain_passive(self):
        self.mutation("Boat/docs/BOAT_R30H_9_PROBE_CROSSWALK.csv",
            "PASSIVE_EXISTING_INTERFACE_ONLY",
            "ACTIVE_DIAGNOSTIC_OUTPUT_NOT_IMPLEMENTED")

if __name__=="__main__":
    unittest.main()
