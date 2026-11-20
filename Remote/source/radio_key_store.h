#pragma once
#include <Arduino.h>

// Security interface. Returns false until a unique key is provisioned in a
// protected storage workflow. No default/test key is embedded in source.
bool radioKey_get(uint8_t keyOut[32]);
