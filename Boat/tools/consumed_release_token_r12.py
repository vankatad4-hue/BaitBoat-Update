"""R12 conceptual consumed/released hardware token temporal model.
This is software verification of a candidate truth machine, NOT a circuit
or evidence of transistor, R/C, race, POR or coil power safety.
"""
from dataclasses import dataclass


@dataclass(frozen=True)
class HardwareInputs:
    request_mask: int = 0
    latch_ok: bool = True
    por_ok: bool = True
    rail_ok: bool = True
    motor_current_zero: bool = True
    coil_current_zero: bool = True


class ConsumedReleaseToken:
    def __init__(self, release_min_ms=50, token_ms=8, input_stable_ms=5):
        if not 1 <= input_stable_ms <= release_min_ms <= 2000:
            raise ValueError("synthetic release timing out of range")
        if not 1 <= token_ms <= release_min_ms:
            raise ValueError("synthetic token length invalid")
        self.release_min_ms=release_min_ms
        self.token_ms=token_ms
        self.input_stable_ms=input_stable_ms
        self.reset()

    def reset(self):
        self.armed=False
        self.fired=False
        self.fault=False
        self.prev_req=False
        self.last_time=None
        self.last_mask=None
        self.mask_since=None
        self.stable_mask=0
        self.release_since=None
        self.token_from=None

    def step(self, now_ms:int, inp:HardwareInputs):
        if not isinstance(now_ms,int) or now_ms<0:
            self.fault=True
        if self.last_time is not None and now_ms<self.last_time:
            self.fault=True
        if not isinstance(inp.request_mask,int) or inp.request_mask not in range(16):
            self.fault=True
        safe=inp.latch_ok and inp.por_ok and inp.rail_ok
        if not safe or self.fault:
            self.reset()
            # A new healthy stable release period must be observed before
            # any new token; an unsafe tick itself does not arm the circuit.
            self.last_time=now_ms if isinstance(now_ms,int) and now_ms>=0 else None
            return False
        if inp.request_mask!=self.last_mask:
            self.last_mask=inp.request_mask
            self.mask_since=now_ms
        if now_ms-self.mask_since>=self.input_stable_ms:
            self.stable_mask=inp.request_mask
        any_req=self.stable_mask!=0
        onehot=self.stable_mask.bit_count()==1
        zeros=inp.motor_current_zero and inp.coil_current_zero
        released=not any_req and zeros
        if released:
            if self.release_since is None:
                self.release_since=now_ms
            if now_ms-self.release_since>=self.release_min_ms and not self.fired:
                self.armed=True
        else:
            self.release_since=None
        if not any_req:
            self.fired=False
        rising=any_req and not self.prev_req
        self.prev_req=any_req
        if rising and onehot and zeros and self.armed and not self.fired:
            self.fired=True
            self.armed=False # consumed on first permitted press
            self.token_from=now_ms
        self.last_time=now_ms
        return (self.fired and self.token_from is not None and
                now_ms-self.token_from<self.token_ms)
