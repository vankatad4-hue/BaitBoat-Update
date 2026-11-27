"""Static, fail-closed drift test for the shared E22 UART implementation.
From repository root, stage source then run:
  python Common/stage.py
  python -m unittest Boat/tests/test_r61_e22_pair_contract.py
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

class SharedEndpointSourceContract(unittest.TestCase):
    def test_endpoint_copies_are_staged_from_one_canonical_driver(self):
        canonical = read("Common/source/e22_uart_link.cpp")
        self.assertEqual(read("Boat/source/e22_uart_link.cpp"), canonical)
        self.assertEqual(read("Remote/source/e22_uart_link.cpp"), canonical)

    def test_shared_uart_configuration(self):
        canonical = read("Common/source/e22_uart_link.cpp")
        for name, expected in {
            "E22_WRITE_CMD": bytes.fromhex("C0 00 09 00 00 00 67 C3 12 00 00 00"),
            "E22_WRITE_ACK": bytes.fromhex("C1 00 09 00 00 00 67 C3 12 00 00 00"),
            "E22_READ_CMD": bytes.fromhex("C1 00 09"),
        }.items():
            with self.subTest(name=name):
                self.assertEqual(config_array(canonical, name), expected)

    def test_command_codec_copies_are_staged_from_common(self):
        canonical = read("Common/source/radio_command_codec.cpp")
        self.assertEqual(read("Boat/source/radio_command_codec.cpp"), canonical)
        self.assertEqual(read("Remote/source/radio_command_codec.cpp"), canonical)

    def test_radio_key_store_interface_is_staged_from_common(self):
        canonical = read("Common/source/radio_key_store.h")
        self.assertEqual(read("Boat/source/radio_key_store.h"), canonical)
        self.assertEqual(read("Remote/source/radio_key_store.h"), canonical)

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
                canonical = read("Common/source/" + name)
                self.assertEqual(read("Boat/source/" + name), canonical)
                self.assertEqual(read("Remote/source/" + name),
                                 canonical)

    def test_declared_matching_model_and_bridge(self):
        manifest = read("Remote/docs/REMOTE_BOAT_DUAL_RADIO_REDUNDANCY_R3.md")
        self.assertIn("identical EBYTE E22-900T22S UART", manifest)
        self.assertIn("SC16IS750", manifest)
        self.assertIn("868.125MHz", manifest)
        self.assertIn("10dBm", manifest)

if __name__ == "__main__":
    unittest.main()
