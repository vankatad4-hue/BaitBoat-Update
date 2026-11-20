#pragma once
#include <SD.h>
#include <stdint.h>
#include <stddef.h>
// Single staged-file copy, called only on the existing firmware loop.
bool mapCopy_start(const char *src,const char *dst,size_t maxSize,size_t exactSize);
int mapCopy_step(uint32_t &copied,uint32_t totalBytes);
bool mapCopy_active();
void mapCopy_cancel();
