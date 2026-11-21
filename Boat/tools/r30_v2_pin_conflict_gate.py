#!/usr/bin/env python3
"""Screenless V2 GPIO design gate. Static conflicts only; NOT electrical verification."""
import unittest

ONBOARD = {
 "sx1262": {8,9,10,11,12,13,14},
 "rf_frontend": {4,5,7},
 "gnss": {33,34,35,36},
 "power_adc": {1,2,3},
 "usb": {19,20},
 "strapping": {0,45,46},
 "crystal": {15,16,47,48},
}
TFT = {"cs":38,"rst":39,"dc":40,"clk":41,"mosi":42,"backlight":21}
UART0 = {43,44}
PROPOSAL = {"nrf_sck":41,"nrf_mosi":42,"nrf_miso":40,
            "nrf_ce":43,"nrf_csn":44,"i2c_sda":6,"i2c_scl":17,
            "motor_pwm_l":21,"motor_pwm_r":37}
def review(pins=PROPOSAL):
    issues=[]
    if len(set(pins.values())) != len(pins): issues.append("duplicate pin assignment")
    for signal,pin in pins.items():
        for owner,reserved in ONBOARD.items():
            if pin in reserved: issues.append(f"{signal} overlaps {owner} GPIO{pin}")
        if pin in TFT.values(): issues.append(f"{signal} shares onboard TFT GPIO{pin}; electrical validation required")
        if pin in UART0: issues.append(f"{signal} uses boot UART0 GPIO{pin}; boot-state validation required")
    if not {"nrf_sck","nrf_mosi","nrf_miso","nrf_ce","nrf_csn","i2c_sda","i2c_scl","motor_pwm_l","motor_pwm_r"}.issubset(pins):
        issues.append("missing required native signal")
    return issues
class GateTests(unittest.TestCase):
    def test_proposal_not_approved(self):
        self.assertTrue(review())
    def test_no_reserved_peripheral(self):
        self.assertFalse(any("overlaps" in x for x in review()))
    def test_motor_backlight_hazard(self):
        self.assertTrue(any("motor_pwm_l shares onboard TFT GPIO21" in x for x in review()))
    def test_uart_boot_hazard(self):
        self.assertEqual(sum("boot UART0" in x for x in review()),2)
    def test_reject_duplicate(self):
        x=dict(PROPOSAL);x["nrf_ce"]=x["nrf_csn"]
        self.assertIn("duplicate pin assignment",review(x))
    def test_reject_rf_conflict(self):
        x=dict(PROPOSAL);x["motor_pwm_r"]=4
        self.assertTrue(any("overlaps rf_frontend" in s for s in review(x)))
if __name__=="__main__": unittest.main()
