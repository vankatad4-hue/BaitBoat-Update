#!/usr/bin/env python3
"""R24 conservative commissioning checklist gate. No hardware I/O. No certification."""
from dataclasses import dataclass, field
import unittest

REQUIRED = (
 "battery_pack_voltage_and_bms","motor_start_and_stall_current",
 "solenoid_peak_and_hold_current","coil_safe_pulse_envelope",
 "stop_monitor_reset_test","contactor_exact_sku_and_aux_feedback",
 "contactor_dc_inductive_rating","contactor_coil_dropout_test",
 "independent_onehot_all_masks","independent_nonretrigger_stuck_high",
 "brownout_por_manual_rearm","independent_current_zero",
 "physical_stop_stuck_output_test","ekm014_vh_vl_pin_test",
 "fuses_wire_connectors_thermal","waterproofing_and_mounting",
 "heltec_v12_gpio_evidence","nrf24_failsafe_test",
)
@dataclass
class Commissioning:
    evidence: dict = field(default_factory=dict)
    hardware_present: bool = False
    reviewed_by_human: bool = False
    sonar_selected: bool = False

def authorize_actuator_commissioning(c: Commissioning):
    errors = [f"missing physical acceptance: {k}" for k in REQUIRED
              if c.evidence.get(k) is not True]
    if not c.hardware_present: errors.append("hardware not physically assembled")
    if not c.reviewed_by_human: errors.append("independent human acceptance absent")
    if c.sonar_selected: errors.append("sonar scope frozen pending owner decision")
    return (len(errors) == 0, errors)

class SafetyTests(unittest.TestCase):
    def test_default_denied(self):
        ok, errors = authorize_actuator_commissioning(Commissioning())
        self.assertFalse(ok); self.assertGreater(len(errors), 10)
    def test_software_assertions_not_enough(self):
        ok, errors = authorize_actuator_commissioning(Commissioning(evidence={k:True for k in REQUIRED}))
        self.assertFalse(ok); self.assertIn("hardware not physically assembled",errors)
    def test_missing_onehot_denied(self):
        e={k:True for k in REQUIRED};e["independent_onehot_all_masks"]=False
        ok, errors = authorize_actuator_commissioning(Commissioning(e,True,True))
        self.assertFalse(ok);self.assertTrue(any("onehot" in x for x in errors))
    def test_no_repeat_denied(self):
        e={k:True for k in REQUIRED};e.pop("independent_nonretrigger_stuck_high")
        self.assertFalse(authorize_actuator_commissioning(Commissioning(e,True,True))[0])
    def test_sonar_scope_denied(self):
        self.assertFalse(authorize_actuator_commissioning(Commissioning({k:True for k in REQUIRED},True,True,True))[0])
    def test_all_synthetic_inputs_only_demonstrate_gate(self):
        self.assertTrue(authorize_actuator_commissioning(Commissioning({k:True for k in REQUIRED},True,True))[0])
if __name__=="__main__":
    unittest.main()
