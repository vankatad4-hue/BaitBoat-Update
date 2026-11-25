#pragma once
// R30H-65: opt-in dual-core scheduler smoke probe, NOT radio migration.
// It only reports task/core affinity and never accesses transport, LVGL,
// GPIO, Wire, SPI, commands or application state.
// Enable with -DFLYTEC_ENABLE_CORE0_PROBE=1 for a diagnostic target build.
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#ifndef FLYTEC_ENABLE_CORE0_PROBE
#define FLYTEC_ENABLE_CORE0_PROBE 0
#endif
namespace flytec_core_probe {
#if FLYTEC_ENABLE_CORE0_PROBE
static_assert(portNUM_PROCESSORS >= 2, "Core-0 probe requires dual-core FreeRTOS");
inline void probeTask(void*) {
 const BaseType_t core=xPortGetCoreID();
 Serial.printf("R30H-65 Core0 probe: core=%d CPUs=%d\\n",
               static_cast<int>(core),portNUM_PROCESSORS);
 vTaskDelete(nullptr);
}
inline bool start() {
 return xTaskCreatePinnedToCore(probeTask,"flytec_core0_probe",2048,
                                nullptr,tskIDLE_PRIORITY+1,nullptr,0)==pdPASS;
}
#else
inline bool start() { return false; }
#endif
} // namespace flytec_core_probe
