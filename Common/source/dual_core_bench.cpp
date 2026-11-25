// R30H-90: standalone bench image, never part of normal application startup.
#if defined(FLYTEC_DUAL_CORE_BENCH) && FLYTEC_DUAL_CORE_BENCH
#include <Arduino.h>
#include "dual_core_diagnostic.h"
static flytec_rtos::DualCoreDiagnostic diagnostic;
void setup() {
 Serial.begin(115200);
 Serial.println("R30H-90 standalone scheduler bench; no application drivers");
}
void loop() {
 static unsigned cycle=0;
 if (!diagnostic.begin()) { Serial.println("FAIL start"); delay(1000); return; }
 delay(1200);
 const auto s=diagnostic.snapshot();
 diagnostic.stop();
 const auto stopped=diagnostic.snapshot();
 const bool pass=s.running && s.core0Ticks>=5 && s.core1Ticks>=5 &&
  s.core0WrongCore==0 && s.core1WrongCore==0 && !stopped.running;
 Serial.printf("cycle=%u c0=%lu c1=%lu wrong0=%lu wrong1=%lu %s\n",
  ++cycle,(unsigned long)s.core0Ticks,(unsigned long)s.core1Ticks,
  (unsigned long)s.core0WrongCore,(unsigned long)s.core1WrongCore,pass?"PASS":"FAIL");
 delay(1000);
}
#endif
