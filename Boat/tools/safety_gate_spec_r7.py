#!/usr/bin/env python3
"""R7 independent hardware power-gate fault latch truth table.

This describes REQUIRED circuit behaviour. It is not the final schematic,
does not power a coil or motor, and cannot replace electrical testing.
"""
from dataclasses import dataclass
from itertools import product

@dataclass
class GateLatch:
    armed: bool=False
    tripped: bool=True
    def apply(self, *, stop_closed, watchdog_ok, uvlo_ok, thermal_ok,
              current_ok, rails_good, local_arm_edge=False):
        safe=all((stop_closed,watchdog_ok,uvlo_ok,thermal_ok,current_ok,rails_good))
        if not safe:
            self.armed=False;self.tripped=True
        elif local_arm_edge and self.tripped:
            self.armed=True;self.tripped=False
        return self.armed and safe

def verify():
    # Every independent fault must remove power without firmware action.
    for combination in product((False,True),repeat=6):
        gate=GateLatch()
        names=("stop_closed","watchdog_ok","uvlo_ok","thermal_ok","current_ok","rails_good")
        values=dict(zip(names,combination))
        enabled=gate.apply(**values,local_arm_edge=True)
        assert enabled == all(combination)
        if not all(combination):
            assert not gate.apply(**{n:True for n in names})
            # A subsequent local arm with all-good inputs is a distinct edge.
            assert gate.apply(**{n:True for n in names},local_arm_edge=True)
        else:
            for n in names:
                v={k:True for k in names};v[n]=False
                assert not gate.apply(**v),n
                assert not gate.apply(**{k:True for k in names})
    # Watchdog's WDO pulse must be remembered after it deasserts.
    g=GateLatch()
    all_ok=dict.fromkeys(("stop_closed","watchdog_ok","uvlo_ok","thermal_ok","current_ok","rails_good"),True)
    assert g.apply(**all_ok,local_arm_edge=True)
    assert not g.apply(**dict(all_ok,watchdog_ok=False))
    assert not g.apply(**all_ok)
    print("64 fault combinations and no-automatic-rearm sequence passed (spec only)")
if __name__=="__main__":
    verify()
