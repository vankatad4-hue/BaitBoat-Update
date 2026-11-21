import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import boat_gpio_candidate_r8 as r


class CandidatePinSafety(unittest.TestCase):
    def test_two_draft_topologies(self):
        self.assertEqual(len(r.SIGNALS), 10)
        self.assertTrue(r.CANDIDATES.isdisjoint(r.RESERVED))
        for name, proposal in r.PROPOSALS.items():
            self.assertTrue(r.validate_proposal(proposal), name)
            self.assertFalse(r.may_energize_actuators(proposal), name)
            self.assertEqual(len(set(proposal.values())), 10)
            self.assertEqual(proposal["independent_gate_permit"], 4)
            self.assertEqual(proposal["external_watchdog_wdi"], 17)

    def test_duplicate_or_reserved_gpio_rejected(self):
        original = dict(r.PROPOSALS["A_LED18"])
        original["nrf_csn"] = original["nrf_ce"]
        self.assertFalse(r.validate_proposal(original))
        original = dict(r.PROPOSALS["A_LED18"])
        original["nrf_ce"] = 8  # Heltec SX1262 NSS, INTERNAL
        self.assertFalse(r.validate_proposal(original))
        original["nrf_ce"] = 33  # GNSS UART
        self.assertFalse(r.validate_proposal(original))
        original["nrf_ce"] = 19  # native USB D-
        self.assertFalse(r.validate_proposal(original))

    def test_missing_and_unsafe_safety_roles_rejected(self):
        p = dict(r.PROPOSALS["A_LED18"])
        del p["external_watchdog_wdi"]
        self.assertFalse(r.validate_proposal(p))
        p = dict(r.PROPOSALS["A_LED18"])
        p["independent_gate_permit"], p["mdd_dir_left"] = (
            p["mdd_dir_left"], p["independent_gate_permit"])
        self.assertFalse(r.validate_proposal(p))
        p = dict(r.PROPOSALS["A_LED18"])
        p["external_watchdog_wdi"], p["mdd_pwm_left"] = (
            p["mdd_pwm_left"], p["external_watchdog_wdi"])
        self.assertFalse(r.validate_proposal(p))

    def test_firmware_configuration_is_not_auto_commissioned(self):
        config = (ROOT / "source" / "boat_radio_hw_config.h").read_text()
        assert "#define BOAT_SX1262_BOARD_AUDIT_SIGNED 0" in config
        assert "#define BOAT_SX1262_PEER_PHY_VERIFIED 0" in config
        assert "#define BOAT_SX1262_EU868_APPROVED 0" in config
        for prop in r.PROPOSALS.values():
            self.assertFalse(r.may_energize_actuators(prop))


if __name__ == "__main__":
    unittest.main()
