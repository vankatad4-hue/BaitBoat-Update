#include "offline_map.h"
#include "lake_storage.h"
#include "offline_map_coverage.h"

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <esp_heap_caps.h>

static String g_map_root;
static OfflineMapStyle g_style=OFFLINE_MAP_STREET;
static bool g_sd_available = false;                 // SD filesystem was mounted by the display adapter.
static bool g_map_pack_available = false;           // Map manifest was found and validated.
static char g_map_attribution[256] = {0};            // Validated map-pack source attribution.

// Require a valid pack header, complete coverage flag and source attribution/licence metadata.
static bool offlineMap_checkManifest(const char *root)
{
    if (!root || !*root) return false;
    File manifest = SD.open((String(root)+"/manifest.txt").c_str(), FILE_READ);
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
    bool hasStyle = false;
    g_map_attribution[0] = '\0';
    char line[256] = {0};
    while (manifest.available()) {
        const size_t count = manifest.readBytesUntil('\n', line, sizeof(line) - 1);
        line[count] = '\0';
        if(count && line[count-1]=='\r')line[count-1]='\0';
        if(strcmp(line,"style=street")==0){g_style=OFFLINE_MAP_STREET;hasStyle=true;}
        if(strcmp(line,"style=satellite")==0){g_style=OFFLINE_MAP_SATELLITE;hasStyle=true;}
        if (strcmp(line, "pack_complete=true") == 0) complete = true;
        if (strncmp(line, "attribution=", 12) == 0 && line[12] != '\0') {
            hasAttribution = true;
            strncpy(g_map_attribution, line + 12, sizeof(g_map_attribution) - 1);
            g_map_attribution[sizeof(g_map_attribution) - 1] = '\0';
        }
        if (strncmp(line, "licence_url=", 12) == 0 && line[12] != '\0') hasLicence = true;
        if (strncmp(line, "zoom_levels=", 12) == 0 && line[12] != '\0') hasZooms = true;
    }
    manifest.close();
    return complete && hasAttribution && hasLicence && hasZooms && hasStyle;
}

// Initialise offline-map storage and validate its map-pack manifest.
bool offlineMap_begin(bool sdCardReady)
{
    g_sd_available = sdCardReady && lakeStorage_ready();                   // Remember the mount result from display_port.cpp.
    g_map_pack_available = false;                   // Clear any previous map-pack status.
    g_map_attribution[0] = '\0';

    if (!g_sd_available)                            // No mounted card means no offline map data can be read.
    {
        Serial.println("MAP: SD storage unavailable; using GPS-only placeholder."); // Explain fallback mode.
        return false;                               // Keep the map grid as a safe visual fallback.
    }

    g_map_root=lakeStorage_mapRoot();
    // Legacy root pack belongs exclusively to Lake 01, until a local pack is installed.
    if(lakeStorage_active()==0 && g_map_root==lakeStorage_path("/maps") && !SD.exists((g_map_root+"/manifest.txt").c_str()) && !SD.exists(lakeStorage_path("/map_slot.bin").c_str()) && !SD.exists(lakeStorage_path("/map_slot.bak").c_str())) g_map_root="/maps";
    g_map_pack_available = offlineMap_checkManifest(g_map_root.c_str()) && mapCoverage_begin(g_map_root.c_str()); // Verify the map-pack format marker.
    Serial.println(g_map_pack_available ? "MAP: Offline map pack found." : "MAP: No offline map pack installed."); // Log status.
    return g_map_pack_available;                    // Return true only when a compatible map pack is present.
}

// Report whether a compatible map pack is installed.
bool offlineMap_hasMapData()
{
    return lakeStorage_ready() && g_map_pack_available;                    // Manifest and coverage index must both validate.
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
    if (!lakeStorage_ready() || !g_sd_available || !g_map_pack_available || style!=g_style || !mapCoverage_tilePresent(zoom,tileX,tileY))   // Require pack/index and indexed tile presence.
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
    snprintf(path, sizeof(path), "%s/%s/%u/%lu/%lu.rgb565", g_map_root.c_str(), styleFolder, static_cast<unsigned>(zoom), static_cast<unsigned long>(tileX), static_cast<unsigned long>(tileY)); // Build tile path.

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

bool offlineMap_validateStaged(const char *root){
    return lakeStorage_ready() && offlineMap_checkManifest(root) && mapCoverage_begin(root);
}
OfflineMapStyle offlineMap_getStyle(){return g_style;}
