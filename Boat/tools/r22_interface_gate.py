#!/usr/bin/env python3
"""R22 host-only conservative hardware interface gates; no GPIO or actuator access."""
from dataclasses import dataclass
import unittest

RESERVED_V11 = frozenset({0,1,2,3,8,9,10,11,12,13,14,15,16,18,19,20,21,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48})
CANDIDATE_V11 = frozenset({4,5,6,7,17,26})
REQUIRED_DIRECT = ("PWM1","PWM2","DIR1","DIR2","NRF_CE","NRF_CSN","I2C_SDA","I2C_SCL")

@dataclass(frozen=True)
class PinEvidence:
    revision: str
    schematic_verified: bool
    physically_verified: bool
    assignments: dict

def gpio_gate(e):
    errors = []
    if e.revision != "v1.2": errors.append("exact v1.2 board revision required")
    if not e.schematic_verified: errors.append("v1.2 schematic evidence missing")
    if not e.physically_verified: errors.append("physical header/boot verification missing")
    for name in REQUIRED_DIRECT:
        if name not in e.assignments: errors.append(f"unassigned {name}")
    pins = list(e.assignments.values())
    if len(pins) != len(set(pins)): errors.append("duplicate GPIO assignment")
    if any(not isinstance(pin,int) or pin<0 or pin>48 for pin in pins):
        errors.append("invalid GPIO")
    # Legacy reservations are conservative until revision-specific evidence.
    if not (e.schematic_verified and e.physically_verified):
        if set(pins) & RESERVED_V11: errors.append("legacy reserved pin used without v1.2 proof")
    return errors

REQUIRED_LORA = ("frequency_hz","bandwidth_hz","spreading_factor","coding_rate",
                 "sync_word","preamble_symbols","crc_enabled","iq_inverted",
                 "header_mode","packet_mode","firmware_framing")

def lora_gate(boat, remote, bidirectional_test=False, authenticated_test=False):
    missing=[k for k in REQUIRED_LORA if k not in boat or k not in remote]
    mismatch=[k for k in REQUIRED_LORA if k in boat and k in remote and boat[k]!=remote[k]]
    errors=[f"missing {k}" for k in missing]+[f"mismatch {k}" for k in mismatch]
    if not bidirectional_test: errors.append("bidirectional physical RF test missing")
    if not authenticated_test: errors.append("authenticated ACK/STOP/failsafe test missing")
    return errors

class GateTests(unittest.TestCase):
    def test_empty_fails_closed(self):
        self.assertTrue(gpio_gate(PinEvidence("v1.2",False,False,{})))
    def test_legacy_not_proof(self):
        e=PinEvidence("v1.1",True,True,dict(zip(REQUIRED_DIRECT,range(4,12))))
        self.assertTrue(gpio_gate(e))
    def test_duplicate_rejected(self):
        self.assertIn("duplicate GPIO assignment",gpio_gate(PinEvidence("v1.2",True,True,{k:4 for k in REQUIRED_DIRECT})))
    def test_missing_radio_fails(self):
        self.assertTrue(lora_gate({},{}))
    def test_same_chip_not_enough(self):
        p={k:"test" for k in REQUIRED_LORA}
        self.assertTrue(lora_gate(p,p))
    def test_mismatch_detected(self):
        a={k:"test" for k in REQUIRED_LORA}
        b=dict(a);b["sync_word"]="different"
        self.assertIn("mismatch sync_word",lora_gate(a,b,True,True))
    def test_complete_test_vector_only(self):
        a={k:"test" for k in REQUIRED_LORA}
        self.assertEqual(lora_gate(a,a,True,True),[])
if __name__ == "__main__":
    unittest.main()
