#!/usr/bin/env python3
"""R28 non-actuating source-level integration invariant checks."""
from pathlib import Path
import csv
ROOT=Path(__file__).resolve().parents[2]
read=lambda p:(ROOT/p).read_text(encoding="utf-8")
r27=read("Boat/source/ina231_voltage_contract.h")
r28=read("Boat/source/ina231_acquisition.h")
wire=read("Boat/source/ina231_arduino_wire.h")
port=read("Boat/source/tracker_sensor_port.cpp")
cfg=read("Boat/source/tracker_sensor_profile.example.h")
hal=read("Boat/source/boat_physical_driver_adapter.h")
assert "INA231_MODULE_SELECTED=true" in r27
assert "INA231_PHYSICAL_COMMISSIONED=false" in r27
assert "CONFIG_BUS_TRIGGERED = 0x4122U" in r28
assert "MASK_CONVERSION_READY = 0x0008U" in r28
assert "MAX_AGE_MS = 1000U" in r28
assert "virtual bool write16" in r28 and "virtual bool read16" in r28
assert "return PollResult::Failed;" in r28
assert "explicit Ina231ArduinoWireR28" in wire
assert "ina231Sampler_.begin(" in port
assert "boat_r27::INA231_PHYSICAL_COMMISSIONED && cfg.pinAuditSigned" in port
assert "cfg.ina231BoardAndWiringSigned && cfg.ina231DmmCalibrationSigned" in port
assert "if(!ina231Ready_){adcPending_=false;voltageValid_=false;return;}" in port
assert "voltageValid_=status!=boat_r28::PollResult::Failed && fresh.valid;" in port
assert "c.ina231BoardAndWiringSigned=false;" in cfg
assert "c.ina231DmmCalibrationSigned=false;" in cfg
assert "!independentHardwareCommissioned" in hal
assert "motorWrite" not in wire and "coilWrite" not in wire and "cutOutputs" not in wire
p=ROOT/"Boat/docs/BOAT_R27_INA231_ACCEPTANCE.csv"
with p.open(newline="",encoding="utf-8") as f:
    rows=list(csv.DictReader(f))
assert len(rows)==21 and all(r["Status"]=="NOT_RUN" for r in rows)
print("R28 pre-prototype invariant PASS: INA231 wired in software, outputs and sensor commissioning remain LOCKED; physical checks 21 NOT_RUN")
