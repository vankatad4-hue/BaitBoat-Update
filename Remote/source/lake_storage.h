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

constexpr size_t LAKE_NAME_BYTES=48;
bool lakeStorage_getName(uint8_t lake,char *name,size_t capacity);
bool lakeStorage_setName(uint8_t lake,const char *name);

// Pin recovery to the currently active pack before overwriting the inactive slot.
bool lakeStorage_prepareMapStage();

uint8_t lakeStorage_getZoom();
bool lakeStorage_setZoom(uint8_t zoom);
