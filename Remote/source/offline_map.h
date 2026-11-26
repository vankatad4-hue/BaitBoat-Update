#ifndef OFFLINE_MAP_H
#define OFFLINE_MAP_H

#include <Arduino.h>

// Two map styles supported by the remote's offline tile pack.
enum OfflineMapStyle : uint8_t
{
    OFFLINE_MAP_STREET = 0,       // Road, shoreline and place-label map.
    OFFLINE_MAP_SATELLITE = 1     // Satellite imagery map.
};

// Raw 256 x 256 RGB565 tile dimensions and byte count.
static constexpr uint16_t OFFLINE_MAP_TILE_SIZE = 256;
static constexpr size_t OFFLINE_MAP_TILE_BYTES = 256UL * 256UL * 2UL;

// Initialise the map catalogue after displayPort_begin() has mounted the SD card.
bool offlineMap_begin(bool sdCardReady);

// True only when manifest, completeness flag, zooms and attribution/licence validate.
bool offlineMap_hasMapData();

// Copy the validated pack attribution into caller storage. Returns false if absent.
bool offlineMap_getAttribution(char *buffer, size_t bufferSize);
void offlineMap_getCoverageStats(uint16_t &planned,uint16_t &present,uint16_t &missing);

// Cooperative single-reader API: step returns -1 error, 0 pending, 1 transferred buffer.
bool offlineMap_startTile(OfflineMapStyle style,uint8_t zoom,uint32_t x,uint32_t y);
int offlineMap_stepTile(uint16_t *&completed);
void offlineMap_cancelTile();

// Release a tile buffer returned by offlineMap_stepTile().
void offlineMap_freeTile(uint16_t *pixels);

// Validate the staged manifest/index before publishing its slot; restore via offlineMap_begin.
bool offlineMap_validateStaged(const char *root);
OfflineMapStyle offlineMap_getStyle();

#endif
