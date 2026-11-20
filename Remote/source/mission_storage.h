#pragma once
#include <Arduino.h>
#include <stddef.h>
constexpr uint8_t SLOT_COUNT = 16;
struct WaypointSlot { bool valid; float lat, lon; bool baitMark, hookMark; };
constexpr uint32_t STORE_MAGIC=0x42544D50UL;
struct StoredMission { uint32_t magic; WaypointSlot points[SLOT_COUNT]; bool homeValid; float homeLat,homeLon; int8_t selected; uint32_t crc; };

uint32_t missionStorage_crc32(const uint8_t *data,size_t len);
bool missionStorage_validCoordinate(float lat,float lon);
bool missionStorage_readRecord(const char *path,StoredMission &record);
bool missionStorage_write(const StoredMission &record);
bool missionStorage_read(StoredMission &record);
// Validate and sanitize mission data before the UI can use it.
void missionStorage_sanitize(StoredMission &record);
