#pragma once
#include <stddef.h>
#include <stdint.h>
constexpr size_t MAP_MANIFEST_LIMIT=2048;
struct MapPackManifest{uint8_t style;uint16_t tileCount;char attribution[256];};
bool mapPack_parseManifest(const char *bytes,size_t length,MapPackManifest &out);
bool mapPack_parseUnsigned(const char *text,unsigned long max,unsigned long &value);
