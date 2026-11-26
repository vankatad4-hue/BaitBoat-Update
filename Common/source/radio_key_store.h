#pragma once
#include <Arduino.h>
// Read the provisioned 32-byte radio key only when secure boot and encrypted
// NVS are enabled; on every failure, clear the output and report unavailable.
bool radioKey_get(uint8_t keyOut[32]);
