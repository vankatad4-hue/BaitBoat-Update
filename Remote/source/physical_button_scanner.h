#pragma once
#include <stdint.h>
typedef void (*PhysicalButtonCallback)(uint8_t buttonId,bool pressed);
void physicalButtons_setCallback(PhysicalButtonCallback);
bool physicalButtons_begin();
void physicalButtons_update(uint32_t nowMs);
uint16_t physicalButtons_state();
