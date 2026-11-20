#pragma once
#include <stdint.h>
extern uint32_t fakeNow;
inline uint32_t millis(){return fakeNow;}
