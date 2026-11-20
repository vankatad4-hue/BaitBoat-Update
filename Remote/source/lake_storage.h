#pragma once
#include <Arduino.h>
constexpr uint8_t LAKE_COUNT = 8;
bool lakeStorage_begin(bool sdReady);
uint8_t lakeStorage_active();
bool lakeStorage_ready();
String lakeStorage_path(const char *relative);
bool lakeStorage_select(uint8_t lake);
