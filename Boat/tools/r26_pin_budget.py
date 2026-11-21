#!/usr/bin/env python3
"""R26 pin-budget feasibility: documentary candidate pins, not approved wiring."""
import unittest

CANDIDATES = frozenset({4,5,6,7,17,26})
# External nRF requires SPI (3) + CSN/CE (2); I2C bus (2), two PWM motor controls.
BASELINE = {"nrf_sck":1,"nrf_mosi":1,"nrf_miso":1,"nrf_csn":1,"nrf_ce":1,
            "i2c_sda":1,"i2c_scl":1,"motor_pwm_left":1,"motor_pwm_right":1}
# Motor DIR, solenoid requests, and STOP feedback are excluded from the baseline,
# and must be addressed separately with qualified COTS hardware.
def feasibility(available=CANDIDATES, shared_spi=False):
    demand = sum(BASELINE.values()) - (3 if shared_spi else 0)
    # Shared SPI is *not* presently an established option: SX1262 bus is
    # connected to GPIO8-11 internally and v1.1 header does not expose it.
    return {"candidate_count":len(available), "minimum_native_gpio":demand,
            "shortfall":max(0,demand-len(available)),
            "qualified":False,
            "reason":"v1.2 schematic and SPI/header evidence missing"}
class Tests(unittest.TestCase):
    def test_nonshared_spi_fails(self):
        self.assertEqual(feasibility()["shortfall"],3)
    def test_shared_spi_still_not_qualified(self):
        self.assertFalse(feasibility(shared_spi=True)["qualified"])
    def test_no_silent_extra_gpio(self):
        self.assertEqual(len(CANDIDATES),6)
    def test_candidate_loss_worsens_budget(self):
        self.assertGreater(feasibility({4,5,6,7,17})["shortfall"],feasibility()["shortfall"])
if __name__=="__main__":unittest.main()
