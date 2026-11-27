from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


def function_body(source: str, signature: str) -> str:
    """Return one C++ function body by balancing its braces."""
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise AssertionError(f"unterminated function: {signature}")


class TestRemoteWorkerActivationLock(unittest.TestCase):
    def test_production_startup_claims_only_the_legacy_owner(self):
        radio = (ROOT / "Remote/source/boat_radio.cpp").read_text()
        setup = (ROOT / "Remote/source/Flytec_Remote.ino").read_text()
        begin = function_body(radio, "bool boatRadio_begin()")

        self.assertIn("if(!g_radioOwner.claimLegacy())return false;", begin)
        self.assertNotIn("claimWorker(", begin)
        self.assertNotIn("g_workerHandshake.start(", begin)
        self.assertNotIn("g_workerTx.begin(", begin)
        self.assertEqual(1, setup.count("boatRadio_begin();"))
        self.assertNotIn("boatRadio_beginWorker(", setup)

    def test_remote_radio_commissioning_inputs_remain_false(self):
        config = (ROOT / "Remote/source/radio_hw_config.h").read_text()
        for flag in (
            "REMOTE_NRF_PIN_AUDIT_SIGNED",
            "REMOTE_NRF_BRIDGE_HARDWARE_AUDIT_SIGNED",
            "REMOTE_NRF_BRIDGE_RF_INTEROPERABILITY_VERIFIED",
            "REMOTE_E22_BRIDGE_AUDIT_SIGNED",
            "REMOTE_E22_RF_INTEROPERABILITY_VERIFIED",
            "REMOTE_E22_RF_AIRTIME_AUDIT_SIGNED",
        ):
            self.assertRegex(config, rf"#define {flag} 0\b")

    def test_boat_production_worker_remains_compile_locked(self):
        header = (ROOT / "Boat/source/boat_dual_radio_port.h").read_text()
        self.assertIn("#define BOAT_R30H121_PHYSICAL_OWNER 0", header)
        self.assertIn(
            "static_assert(BOAT_R30H121_PHYSICAL_OWNER == 0", header
        )


if __name__ == "__main__":
    unittest.main()
