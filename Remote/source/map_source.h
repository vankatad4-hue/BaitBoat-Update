#ifndef MAP_SOURCE_H
#define MAP_SOURCE_H

#include <Arduino.h>

// User preference for map source; online imagery is not integrated yet.
enum MapSourceMode : uint8_t {
    MAP_SOURCE_AUTO = 0,
    MAP_SOURCE_ONLINE = 1,
    MAP_SOURCE_OFFLINE = 2
};

void mapSource_begin();
MapSourceMode mapSource_getMode();
void mapSource_setMode(MapSourceMode mode);
MapSourceMode mapSource_nextMode();
const char *mapSource_modeLabel(MapSourceMode mode);
// Returns whether the requested online provider can actually render at this build.
bool mapSource_onlineAvailable();

#endif
