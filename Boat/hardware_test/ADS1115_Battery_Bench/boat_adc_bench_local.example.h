#pragma once
// R22 EXAMPLE ONLY — do not include this file automatically.
// Make a PRIVATE copy named boat_adc_bench_local.h beside the .ino,
// after inspecting the exact purchased DevKitC revision and physically
// separating all actuator supplies.
//
// 1 = the operator has signed the physical pin/power audit externally.
// NOTE: signing this flag does not constitute a completed hardware test.
#define BOAT_ADC_BENCH_PIN_AUDIT_SIGNED 0

// R20 candidates GPIO8 SDA and GPIO7 SCL; change only after a documented
// hardware map revision (the sketch rejects other assignments).
#define BOAT_ADC_BENCH_SDA_GPIO 8
#define BOAT_ADC_BENCH_SCL_GPIO 7
