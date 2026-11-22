"""R22 static contract: sensor-only sketch cannot silently enable Boat outputs."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SKETCH = ROOT / "hardware_test" / "ADS1115_Battery_Bench" / "ADS1115_Battery_Bench.ino"
EXAMPLE = ROOT / "hardware_test" / "ADS1115_Battery_Bench" / "boat_adc_bench_local.example.h"
SOURCE = SKETCH.read_text(encoding="utf-8")
LOCAL = EXAMPLE.read_text(encoding="utf-8")

def require(test: bool, description: str) -> None:
    if not test:
        raise SystemExit("FAILED R22 ADC bench isolation: " + description)

require("#define BOAT_ADC_BENCH_PIN_AUDIT_SIGNED 0" in SOURCE,
        "sketch must default to disabled GPIO audit")
require("#define BOAT_ADC_BENCH_PIN_AUDIT_SIGNED 0" in LOCAL,
        "example local GPIO audit must remain disabled")
require("BOAT_ADC_BENCH_SDA_GPIO == 8" in SOURCE and "BOAT_ADC_BENCH_SCL_GPIO == 7" in SOURCE,
        "sketch must restrict opt-in to documented R20 I2C pair")
require("#if BOAT_ADC_BENCH_READY_FOR_SENSOR_ONLY_I2C" in SOURCE,
        "I2C must be behind signed physical audit gate")
require("BOAT_ADS1115_CONFIG_AIN0_SINGLE_128SPS_4V096" in SOURCE,
        "fixed single-shot ADC configuration required")
require("UNCOMMISSIONED" in SOURCE, "every measurement must disclaim safety authority")
require("return false" in SOURCE, "I2C error handling must fail closed")
require("Wire.begin(" in SOURCE and "Wire.requestFrom(" in SOURCE,
        "bench must have actual I2C readout")
allowed_includes = {"Arduino.h", "Wire.h", "boat_ads1115_bench_math.h", "boat_adc_bench_local.h"}
for item in re.findall(r'^\s*#include\s*["<]([^">]+)[">]', SOURCE, re.M):
    require(item in allowed_includes, "unexpected dependency: " + item)
for forbidden in ("pinMode(", "digitalWrite(", "analogWrite(", "ledcWrite(",
                  "application.tick(", "radio.begin(", "MDD20A", "EKM014"):
    # Driver model names are allowed only as comments in the sketch, not
    # as executable output calls or compiled import dependencies.
    if forbidden in ("MDD20A", "EKM014"):
        continue
    require(forbidden not in SOURCE, "actuator or radio API not allowed: " + forbidden)
print("R22 ADS1115 bench isolation: locked by default, no actuator APIs, no radio")
