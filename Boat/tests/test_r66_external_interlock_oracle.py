"""R66 external-interlock BEHAVIOURAL TEST ORACLE ONLY, NEVER embedded Boat firmware.
Pure host reference for expected black-box acceptance. This is not a hardware
controller, Pilz program, current measurement, safety certification or approval.
40ms/10ms below are test fixture values NOT acceptable coil-pulse limits.
"""
import unittest

class ExternalInterlockOracle:
    """Host-only finite-state specification for a future independent module."""
    def __init__(self, max_ms=40, neutral_ms=10):
        self.max_ms, self.neutral_ms = max_ms, neutral_ms
        self.state = "LATCHED"
        self.output = 0
        self.active = 0
        self.started = 0
        self.quiet_since = 0
        self.last_time = None
        self.last_reset = False

    def tick(self, time_ms, requests=0, health_ok=True, all_currents_zero=True,
             local_reset=False):
        edge = local_reset and not self.last_reset
        self.last_reset = local_reset
        invalid = (self.last_time is not None and time_ms < self.last_time) or requests < 0 or requests > 15
        self.last_time = time_ms
        self.output = 0   # default denied on every sampled tick
        if invalid or not health_ok:
            self.state = "FAULT"
            return 0
        if self.state in ("LATCHED", "FAULT"):
            # No automatic rearm after power return or held physical reset.
            if edge and requests == 0 and all_currents_zero:
                self.state = "ARMED"
            return 0
        if requests.bit_count() > 1:
            self.state = "FAULT"
            return 0
        if self.state == "ARMED":
            if not all_currents_zero:
                self.state = "FAULT"
            elif requests:
                self.active = requests
                self.started = time_ms
                self.state = "PULSE"
                self.output = requests
        elif self.state == "PULSE":
            if requests not in (0, self.active):
                self.state = "FAULT"
            elif requests == 0 or time_ms - self.started >= self.max_ms:
                self.state = "WAIT_ZERO"
            else:
                self.output = self.active
        elif self.state == "WAIT_ZERO":
            if requests == 0 and all_currents_zero:
                self.quiet_since = time_ms
                self.state = "QUIET"
        elif self.state == "QUIET":
            if requests or not all_currents_zero:
                self.state = "WAIT_ZERO"
            elif time_ms - self.quiet_since >= self.neutral_ms:
                self.state = "ARMED"
        else:
            self.state = "FAULT"
        return self.output

class R66BlackBoxAcceptanceOracle(unittest.TestCase):
    @staticmethod
    def armed():
        c = ExternalInterlockOracle()
        assert c.tick(0, 0, local_reset=True) == 0
        assert c.state == "ARMED"
        return c

    def test_all_power_on_masks_denied_without_local_reset(self):
        for mask in range(16):
            c = ExternalInterlockOracle()
            self.assertEqual(c.tick(0, mask), 0)
            self.assertEqual(c.tick(10, mask, local_reset=True), 0)
            self.assertEqual(c.state, "ARMED" if mask == 0 else "LATCHED")

    def test_four_channels_bounded_single_event_no_hold_repeat(self):
        for bit in range(4):
            with self.subTest(bit=bit):
                c = self.armed()
                self.assertEqual(c.tick(1, 1 << bit), 1 << bit)
                self.assertEqual(c.tick(39, 1 << bit), 1 << bit)
                self.assertEqual(c.tick(41, 1 << bit), 0)
                self.assertEqual(c.state, "WAIT_ZERO")
                for t in (80, 100, 1000, 10000):
                    self.assertEqual(c.tick(t, 1 << bit), 0)
                c.tick(10001, 0)
                self.assertEqual(c.state, "QUIET")
                self.assertEqual(c.tick(10011, 0), 0)
                self.assertEqual(c.state, "ARMED")
                self.assertEqual(c.tick(10012, 1 << bit), 1 << bit)

    def test_simultaneous_or_channel_change_latches_fault(self):
        for mask in range(1, 16):
            if mask.bit_count() > 1:
                c = self.armed()
                self.assertEqual(c.tick(1, mask), 0)
                self.assertEqual(c.state, "FAULT")
        c = self.armed()
        c.tick(1, 1)
        self.assertEqual(c.tick(2, 2), 0)
        self.assertEqual(c.state, "FAULT")

    def test_current_zero_and_neutral_guard(self):
        c = self.armed()
        c.tick(1, 1)
        self.assertEqual(c.tick(2, 0, all_currents_zero=False), 0)
        self.assertEqual(c.tick(100, 0, all_currents_zero=False), 0)
        self.assertEqual(c.state, "WAIT_ZERO")
        self.assertEqual(c.tick(101, 0), 0)
        self.assertEqual(c.tick(110, 0), 0)
        self.assertEqual(c.state, "QUIET")
        self.assertEqual(c.tick(111, 0), 0)
        self.assertEqual(c.state, "ARMED")

    def test_watchdog_stop_brownout_or_stale_contact_never_auto_rearm(self):
        for fault_name in ("STOP_OPEN", "WATCHDOG_LOST", "BROWNOUT", "CONTACTOR_WELD", "SENSOR_STALE"):
            with self.subTest(fault=fault_name):
                c = self.armed()
                self.assertEqual(c.tick(1, 1), 1)
                self.assertEqual(c.tick(2, 1, health_ok=False, local_reset=True), 0)
                self.assertEqual(c.state, "FAULT")
                self.assertEqual(c.tick(100, 0, health_ok=True, local_reset=True), 0)
                self.assertEqual(c.state, "FAULT")
                self.assertEqual(c.tick(101, 0, local_reset=False), 0)
                self.assertEqual(c.tick(102, 0, local_reset=True), 0)
                self.assertEqual(c.state, "ARMED")

    def test_clock_rollback_is_fault(self):
        c = self.armed()
        self.assertEqual(c.tick(2, 1), 1)
        self.assertEqual(c.tick(1, 1), 0)
        self.assertEqual(c.state, "FAULT")

if __name__ == "__main__":
    unittest.main()
