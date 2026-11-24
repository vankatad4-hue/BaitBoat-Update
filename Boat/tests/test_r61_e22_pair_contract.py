"""Static, fail-closed drift test for the existing E22 UART radio pair.
Run from repository root: python -m unittest Boat/tests/test_r61_e22_pair_contract.py
This does NOT establish RF or regulatory compliance.
"""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[2]

def read(relative):
    return (ROOT / relative).read_text(encoding="utf-8")

def config_array(source, name):
    match = re.search(r"static\s+const\s+uint8_t\s+" + re.escape(name) +
                      r"\s*\[\s*\d+\s*\]\s*=\s*\{([^}]+)\}", source)
    if not match:
        raise AssertionError(f"Missing {name} E22 config constant")
    return bytes(int(x.strip(), 0) for x in match.group(1).split(",") if x.strip())

class E22PairContract(unittest.TestCase):
    def test_identical_uart_configuration(self):
        remote = read("Remote/source/e22_uart_link.cpp")
        boat = read("Boat/source/e22_uart_link.cpp")
        for name, expected in {
            "E22_WRITE_CMD": bytes.fromhex("C0 00 09 00 00 00 67 C3 12 00 00 00"),
            "E22_WRITE_ACK": bytes.fromhex("C1 00 09 00 00 00 67 C3 12 00 00 00"),
            "E22_READ_CMD": bytes.fromhex("C1 00 09"),
        }.items():
            with self.subTest(name=name):
                self.assertEqual(config_array(remote, name), expected)
                self.assertEqual(config_array(boat, name), expected)

    def test_rx_capacity_guard_and_diagnostic(self):
        # Source contract only; executable Stream mock coverage remains required.
        for endpoint in ("Boat", "Remote"):
            with self.subTest(endpoint=endpoint):
                source = read(endpoint + "/source/e22_uart_link.cpp")
                self.assertIn("while(processed<32U && rxQueueCount_<4U && uart_.available())", source)
                self.assertIn("if(rxQueueCount_==4U && rxFullPollCount_!=UINT32_MAX)", source)
                self.assertIn("++rxFullPollCount_", source)
                header = read(endpoint + "/source/e22_uart_link.h")
                self.assertIn("rxFullPollCount() const", header)

    def test_equal_application_wire_contract(self):
        for name in ("radio_protocol.h", "radio_protocol.cpp"):
            with self.subTest(name=name):
                self.assertEqual(read("Remote/source/" + name),
                                 read("Boat/source/" + name))

    def test_declared_matching_model_and_bridge(self):
        manifest = read("Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md")
        self.assertIn("identical EBYTE E22-900T22S UART", manifest)
        self.assertIn("SC16IS750", manifest)
        self.assertIn("868.125MHz", manifest)
        self.assertIn("10dBm", manifest)

if __name__ == "__main__":
    unittest.main()
