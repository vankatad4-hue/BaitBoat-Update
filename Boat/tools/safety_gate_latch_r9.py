#!/usr/bin/env python3
"""R9 proposed HARDWARE fault-latch/pulse-limiter behavior, host test only.
A Python state machine CANNOT substitute for the independent physical circuit.
"""
from dataclasses import dataclass


@dataclass(frozen=True)
class InterlockSample:
    stop_closed: bool
    watchdog_ok: bool
    uvlo_ok: bool
    thermal_ok: bool
    current_ok: bool
    rails_good: bool
    motors_idle: bool
    solenoids_idle: bool
    output_feedback_valid: bool

    def fault_free(self):
        return all((self.stop_closed, self.watchdog_ok, self.uvlo_ok,
                    self.thermal_ok, self.current_ok, self.rails_good,
                    self.output_feedback_valid))

    def safe_for_rearm(self):
        return self.fault_free() and self.motors_idle and self.solenoids_idle


class PhysicalFaultLatchModel:
    """Power-up OFF, fault-latching, physical re-arm edge after healthy release."""
    def __init__(self):
        self.tripped = True
        self._last_arm_pressed = False
        self._release_seen_while_healthy = False

    def step(self, fb: InterlockSample, *, physical_arm_pressed: bool,
             mcu_permit: bool) -> bool:
        # Software never clears a hardware fault. MCU permit only removes power.
        healthy = fb.fault_free()
        ready = fb.safe_for_rearm()
        pressed = bool(physical_arm_pressed)
        rising = pressed and not self._last_arm_pressed
        self._last_arm_pressed = pressed
        if not healthy:
            self.tripped = True
            self._release_seen_while_healthy = False
            return False
        if not pressed and ready:
            self._release_seen_while_healthy = True
        if self.tripped and rising and ready and self._release_seen_while_healthy:
            self.tripped = False
            self._release_seen_while_healthy = False
        return not self.tripped and bool(mcu_permit)


class OneHotPulseLimiterModel:
    """Abstract external mono-shot behavior: no MCU autonomous re-triggering.

    limit_ms is a simulated hypothetical test setting, not a selected hardware RC.
    Limits should be specified for each actual coil after characterization.
    """
    def __init__(self, limit_ms: int):
        if not 1 <= limit_ms <= 3999:
            raise ValueError("synthetic pulse timeout invalid")
        self.max_ms = limit_ms
        self.active_channel = -1
        self.at = 0
        self.needs_release = False
        self.fault = False

    def reset(self):
        self.__init__(self.max_ms)

    def step(self, now_ms: int, request_mask: int, *,
             gate_power_good: bool, motors_idle: bool,
             coils_zero_current: bool) -> int:
        # A real monostable has its own clock. A simulated backwards timestamp
        # is not a valid wrap unless an explicit 32-bit modular counter is used.
        # Fail closed on a time discontinuity instead of extending a coil pulse.
        if not isinstance(now_ms, int) or now_ms < 0:
            self.fault = True
        if self.active_channel >= 0 and now_ms < self.at:
            self.fault = True
        if not gate_power_good:
            self.fault = True
        if request_mask < 0 or request_mask > 15 or request_mask.bit_count() > 1:
            self.fault = True
        if not motors_idle:
            self.fault = True
        if self.fault:
            self.active_channel = -1
            return 0
        # A pulse holds exactly one channel; a changed or dropped request
        # immediately ceases drive. A stuck high terminates at the timeout.
        if self.active_channel >= 0:
            bit = 1 << self.active_channel
            if request_mask != bit or now_ms - self.at >= self.max_ms:
                self.active_channel = -1
                self.needs_release = True
                return 0
            return bit
        if self.needs_release:
            if request_mask == 0 and coils_zero_current:
                self.needs_release = False
            return 0
        if request_mask == 0:
            return 0
        if not coils_zero_current:
            self.fault = True
            return 0
        self.active_channel = request_mask.bit_length() - 1
        self.at = now_ms
        return request_mask


def safe_example():
    return InterlockSample(True, True, True, True, True, True,
                           True, True, True)
