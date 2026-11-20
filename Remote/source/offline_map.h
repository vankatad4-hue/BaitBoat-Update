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

// Load one raw RGB565 tile into PSRAM. Caller must release it with offlineMap_freeTile().
// Tile path: /maps/street/<z>/<x>/<y>.rgb565 or /maps/satellite/<z>/<x>/<y>.rgb565
uint16_t *offlineMap_readTile(OfflineMapStyle style, uint8_t zoom, uint32_t tileX, uint32_t tileY);

// Release a tile buffer allocated by offlineMap_readTile().
void offlineMap_freeTile(uint16_t *pixels);

#endif
