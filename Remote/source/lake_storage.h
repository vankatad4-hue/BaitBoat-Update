#pragma once
#include <Arduino.h>
constexpr uint8_t LAKE_COUNT = 8;
bool lakeStorage_begin(bool sdReady);
uint8_t lakeStorage_active();
bool lakeStorage_ready();
String lakeStorage_path(const char *relative);
bool lakeStorage_select(uint8_t lake);

// A/B map directories: only the committed slot is visible to readers.
String lakeStorage_mapRoot();
String lakeStorage_mapStage();
bool lakeStorage_commitMap(const String &stagedRoot);
bool lakeStorage_makeDirectories(const char *path);
