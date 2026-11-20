#include "offline_map.h"
#include "offline_map_coverage.h"

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <esp_heap_caps.h>

static bool g_sd_available = false;                 // SD filesystem was mounted by the display adapter.
static bool g_map_pack_available = false;           // Map manifest was found and validated.

// Require a valid pack header, complete coverage flag and source attribution/licence metadata.
static bool offlineMap_checkManifest()
{
    File manifest = SD.open("/maps/manifest.txt", FILE_READ);
    if (!manifest) return false;

    char signature[12] = {0};
    const size_t bytesRead = manifest.read(reinterpret_cast<uint8_t *>(signature), 11);
    if (bytesRead != 11 || strncmp(signature, "FLYTEC_MAP1", 11) != 0) {
        manifest.close();
        return false;
    }

    bool complete = false;
    bool hasAttribution = false;
    bool hasLicence = false;
    bool hasZooms = false;
    g_map_attribution[0] = '\0';
    char line[256] = {0};
    while (manifest.available()) {
        const size_t count = manifest.readBytesUntil('\n', line, sizeof(line) - 1);
        line[count] = '\0';
        if (strncmp(line, "pack_complete=true", 18) == 0) complete = true;
        if (strncmp(line, "attribution=", 12) == 0 && line[12] != '\0') {
            hasAttribution = true;
            strncpy(g_map_attribution, line + 12, sizeof(g_map_attribution) - 1);
            g_map_attribution[sizeof(g_map_attribution) - 1] = '\0';
        }
        if (strncmp(line, "licence_url=", 12) == 0 && line[12] != '\0') hasLicence = true;
        if (strncmp(line, "zoom_levels=", 12) == 0 && line[12] != '\0') hasZooms = true;
    }
    manifest.close();
    return complete && hasAttribution && hasLicence && hasZooms;
}

// Initialise offline-map storage and validate its map-pack manifest.
bool offlineMap_begin(bool sdCardReady)
{
    g_sd_available = sdCardReady;                   // Remember the mount result from display_port.cpp.
    g_map_pack_available = false;                   // Clear any previous map-pack status.
    g_map_attribution[0] = '\0';

    if (!g_sd_available)                            // No mounted card means no offline map data can be read.
    {
        Serial.println("MAP: SD storage unavailable; using GPS-only placeholder."); // Explain fallback mode.
        return false;                               // Keep the map grid as a safe visual fallback.
    }

    g_map_pack_available = offlineMap_checkManifest() && mapCoverage_begin(); // Verify the map-pack format marker.
    Serial.println(g_map_pack_available ? "MAP: Offline map pack found." : "MAP: No offline map pack installed."); // Log status.
    return g_map_pack_available;                    // Return true only when a compatible map pack is present.
}

// Report whether a compatible map pack is installed.
bool offlineMap_hasMapData()
{
    return g_map_pack_available;                    // Manifest and coverage index must both validate.
}

// Copy the validated attribution string to a caller-owned buffer.
bool offlineMap_getAttribution(char *buffer, size_t bufferSize)
{
    if (buffer == nullptr || bufferSize == 0 || !g_map_pack_available || g_map_attribution[0] == '\0')
        return false;
    strncpy(buffer, g_map_attribution, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
    return true;
}

void offlineMap_getCoverageStats(uint16_t &planned,uint16_t &present,uint16_t &missing){mapCoverage_getStats(planned,present,missing);}

// Read one raw RGB565 tile from the selected map style and zoom level.
uint16_t *offlineMap_readTile(OfflineMapStyle style, uint8_t zoom, uint32_t tileX, uint32_t tileY)
{
    if (!g_sd_available || !g_map_pack_available || !mapCoverage_tilePresent(zoom,tileX,tileY))   // Require pack/index and indexed tile presence.
    {
        return nullptr;                              // No tile can be loaded safely.
    }

    if (zoom > 22 || tileX >= (1UL << zoom) || tileY >= (1UL << zoom)) // Reject invalid Web-Mercator tile coordinates.
    {
        return nullptr;                              // Avoid constructing an invalid path.
    }
    if (style != OFFLINE_MAP_STREET && style != OFFLINE_MAP_SATELLITE) // Reject unknown map-style enum values.
    {
        return nullptr;                              // Do not silently reinterpret invalid values as street tiles.
    }

    const char *styleFolder = (style == OFFLINE_MAP_SATELLITE) ? "satellite" : "street"; // Select style directory.
    char path[96];                                  // Hold the full tile path without dynamic String allocation.
    snprintf(path, sizeof(path), "/maps/%s/%u/%lu/%lu.rgb565", styleFolder, static_cast<unsigned>(zoom), static_cast<unsigned long>(tileX), static_cast<unsigned long>(tileY)); // Build tile path.

    File tileFile = SD.open(path, FILE_READ);        // Open the requested tile in read-only mode.
    if (!tileFile)                                  // A missing tile is not replaced with a fake land/water image.
    {
        return nullptr;                              // Let the renderer show its neutral missing-tile background.
    }

    if (tileFile.size() != OFFLINE_MAP_TILE_BYTES)   // Every tile must be exactly 256x256 RGB565 pixels.
    {
        tileFile.close();                            // Close malformed or unsupported tile files.
        return nullptr;                              // Reject partial, compressed or wrong-format tiles.
    }

    uint16_t *pixels = static_cast<uint16_t *>(heap_caps_malloc(OFFLINE_MAP_TILE_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)); // Allocate tile memory in PSRAM.
    if (pixels == nullptr)                          // Check that the 128 KiB allocation succeeded.
    {
        tileFile.close();                            // Close the file before returning.
        return nullptr;                              // The caller can retry after cache eviction.
    }

    const size_t bytesRead = tileFile.read(reinterpret_cast<uint8_t *>(pixels), OFFLINE_MAP_TILE_BYTES); // Read the complete tile.
    tileFile.close();                                // Release the filesystem handle promptly.

    if (bytesRead != OFFLINE_MAP_TILE_BYTES)        // Reject short reads caused by damaged media or I/O errors.
    {
        heap_caps_free(pixels);                      // Return the incomplete tile buffer to PSRAM.
        return nullptr;                              // Never display incomplete image data.
    }

    return pixels;                                  // Caller owns the PSRAM tile until offlineMap_freeTile().
}

// Release a tile buffer after the renderer no longer references it.
void offlineMap_freeTile(uint16_t *pixels)
{
    if (pixels != nullptr)                           // Ignore null pointers safely.
    {
        heap_caps_free(pixels);                      // Return the tile buffer to PSRAM.
    }
}
