#include "map_source.h"
#include <Preferences.h>

static MapSourceMode selectedMode = MAP_SOURCE_AUTO;
static bool initialized = false;

void mapSource_begin()
{
    Preferences prefs;
    if (prefs.begin("map-source", true)) {
        uint8_t stored = prefs.getUChar("mode", MAP_SOURCE_AUTO);
        prefs.end();
        selectedMode = (stored <= MAP_SOURCE_OFFLINE) ? (MapSourceMode)stored : MAP_SOURCE_AUTO;
    } else {
        selectedMode = MAP_SOURCE_AUTO;
    }
    initialized = true;
}

MapSourceMode mapSource_getMode()
{
    if (!initialized) mapSource_begin();
    return selectedMode;
}

void mapSource_setMode(MapSourceMode mode)
{
    if (mode > MAP_SOURCE_OFFLINE) mode = MAP_SOURCE_AUTO;
    selectedMode = mode;
    Preferences prefs;
    if (prefs.begin("map-source", false)) {
        prefs.putUChar("mode", (uint8_t)mode);
        prefs.end();
    }
    initialized = true;
}

MapSourceMode mapSource_nextMode()
{
    MapSourceMode next = (MapSourceMode)(((uint8_t)mapSource_getMode() + 1U) % 3U);
    mapSource_setMode(next);
    return next;
}

const char *mapSource_modeLabel(MapSourceMode mode)
{
    switch (mode) {
        case MAP_SOURCE_ONLINE: return "ONLINE";
        case MAP_SOURCE_OFFLINE: return "OFFLINE";
        default: return "AUTO";
    }
}

// Deliberately false until an authorized online provider adapter is implemented.
bool mapSource_onlineAvailable()
{
    return false;
}
