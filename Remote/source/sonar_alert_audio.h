#pragma once
#include <stdint.h>

// A passive piezo can reproduce the synthesized chirp without storing audio.
// The output remains unavailable until a Remote GPIO is physically assigned.
void sonarAudio_setFishAlarmEnabled(bool enabled);
void sonarAudio_notifyEchoCandidate();
void sonarAudio_poll(uint32_t nowMs);
bool sonarAudio_outputAvailable();
