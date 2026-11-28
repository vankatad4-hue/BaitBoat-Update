#include "sonar_alert_audio.h"
#include <Arduino.h>

// GPIO and output are deliberately uncommissioned. Set both only after the
// buzzer connector/pin, drive circuit, and logic levels are verified.
#ifndef REMOTE_SONAR_BUZZER_ENABLED
#define REMOTE_SONAR_BUZZER_ENABLED 0
#endif
#ifndef REMOTE_SONAR_BUZZER_PIN
#define REMOTE_SONAR_BUZZER_PIN (-1)
#endif
static_assert(REMOTE_SONAR_BUZZER_ENABLED == 0 || REMOTE_SONAR_BUZZER_PIN >= 0,
              "Enabled piezo output requires an assigned GPIO");

namespace {
bool alarmEnabled = false;
bool toneActive = false;
uint32_t toneStartedAtMs = 0U;
}

bool sonarAudio_outputAvailable() {
  return REMOTE_SONAR_BUZZER_ENABLED == 1 && REMOTE_SONAR_BUZZER_PIN >= 0;
}

void sonarAudio_setFishAlarmEnabled(bool enabled) {
  alarmEnabled = enabled;
}

void sonarAudio_notifyEchoCandidate() {
#if REMOTE_SONAR_BUZZER_ENABLED
  if (!alarmEnabled) return;
  if (ledcAttach(REMOTE_SONAR_BUZZER_PIN, 1500U, 10U)) {
    ledcWriteTone(REMOTE_SONAR_BUZZER_PIN, 1450U);
    toneStartedAtMs = millis();
    toneActive = true;
  }
#endif
}

void sonarAudio_poll(uint32_t nowMs) {
#if REMOTE_SONAR_BUZZER_ENABLED
  if (toneActive && static_cast<uint32_t>(nowMs - toneStartedAtMs) >= 130U) {
    ledcWriteTone(REMOTE_SONAR_BUZZER_PIN, 0U);
    ledcDetach(REMOTE_SONAR_BUZZER_PIN);
    toneActive = false;
  }
#else
  (void)nowMs;
#endif
}
