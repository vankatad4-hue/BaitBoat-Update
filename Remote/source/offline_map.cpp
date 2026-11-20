#include "offline_map.h"
#include "map_pack_manifest.h"
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
static bool offlineMap_checkManifest(const char *root){
    if(!root||!*root)return false;
    File file=SD.open((String(root)+"/manifest.txt").c_str(),FILE_READ);
    if(!file)return false;
    char data[MAP_MANIFEST_LIMIT];size_t n=file.size();
    bool readOk=n<sizeof(data)&&file.read(reinterpret_cast<uint8_t*>(data),n)==n;
    file.close();
    MapPackManifest parsed={};
    if(!readOk||!mapPack_parseManifest(data,n,parsed))return false;
    if(!mapCoverage_begin(root))return false;
    uint16_t planned,present,missing;mapCoverage_getStats(planned,present,missing);
    if(planned!=parsed.tileCount||present!=planned||missing){mapCoverage_begin(nullptr);return false;}
    g_style=parsed.style?OFFLINE_MAP_SATELLITE:OFFLINE_MAP_STREET;
    memcpy(g_map_attribution,parsed.attribution,sizeof(g_map_attribution));return true;
}

// Initialise offline-map storage and validate its map-pack manifest.
bool offlineMap_begin(bool sdCardReady)
{
    offlineMap_cancelTile();
    g_sd_available = sdCardReady && lakeStorage_ready();                   // Remember the mount result from display_port.cpp.
    g_map_pack_available = false;                   // Clear any previous map-pack status.
    g_map_attribution[0] = '\0';
    mapCoverage_begin(nullptr);

    if (!g_sd_available)                            // No mounted card means no offline map data can be read.
    {
        Serial.println("MAP: SD storage unavailable; using GPS-only placeholder."); // Explain fallback mode.
        return false;                               // Keep the map grid as a safe visual fallback.
    }

    g_map_root=lakeStorage_mapRoot();
    // Legacy root pack belongs exclusively to Lake 01, until a local pack is installed.
    if(lakeStorage_active()==0 && g_map_root==lakeStorage_path("/maps") && !SD.exists((g_map_root+"/manifest.txt").c_str()) && !SD.exists(lakeStorage_path("/map_slot.bin").c_str()) && !SD.exists(lakeStorage_path("/map_slot.bak").c_str())) g_map_root="/maps";
    g_map_pack_available = offlineMap_checkManifest(g_map_root.c_str()); // Verify the map-pack format marker.
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
    if (buffer == nullptr || bufferSize == 0 || !offlineMap_hasMapData() || g_map_attribution[0] == '\0')
        return false;
    strncpy(buffer, g_map_attribution, bufferSize - 1);
    buffer[bufferSize - 1] = '\0';
    return true;
}

void offlineMap_getCoverageStats(uint16_t &planned,uint16_t &present,uint16_t &missing){mapCoverage_getStats(planned,present,missing);}

// One cooperative reader; all calls occur on the existing UI/control loop.
static File tileInput;
static uint16_t *tileBuffer=nullptr;
static size_t tileOffset=0;
void offlineMap_cancelTile(){
    if(tileInput)tileInput.close();
    if(tileBuffer)heap_caps_free(tileBuffer);
    tileBuffer=nullptr;tileOffset=0;
}
bool offlineMap_startTile(OfflineMapStyle style,uint8_t zoom,uint32_t x,uint32_t y){
    offlineMap_cancelTile();
    if(!offlineMap_hasMapData()||style!=g_style||zoom>22||
       x>=(1UL<<zoom)||y>=(1UL<<zoom)||!mapCoverage_tilePresent(zoom,x,y))return false;
    char path[96];
    int len=snprintf(path,sizeof(path),"%s/%s/%u/%lu/%lu.rgb565",g_map_root.c_str(),
        style==OFFLINE_MAP_SATELLITE?"satellite":"street",(unsigned)zoom,(unsigned long)x,(unsigned long)y);
    if(len<0||(size_t)len>=sizeof(path))return false;
    tileInput=SD.open(path,FILE_READ);
    if(!tileInput||tileInput.size()!=OFFLINE_MAP_TILE_BYTES){offlineMap_cancelTile();return false;}
    tileBuffer=static_cast<uint16_t*>(heap_caps_malloc(OFFLINE_MAP_TILE_BYTES,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
    if(!tileBuffer){offlineMap_cancelTile();return false;}
    return true;
}
int offlineMap_stepTile(uint16_t *&completed){
    completed=nullptr;
    if(!tileBuffer||!tileInput||!offlineMap_hasMapData()){offlineMap_cancelTile();return -1;}
    constexpr size_t chunk=4096;
    size_t remaining=OFFLINE_MAP_TILE_BYTES-tileOffset;
    size_t count=remaining<chunk?remaining:chunk;
    if(tileInput.read(reinterpret_cast<uint8_t*>(tileBuffer)+tileOffset,count)!=count){
        offlineMap_cancelTile();return -1;
    }
    tileOffset+=count;
    if(tileOffset!=OFFLINE_MAP_TILE_BYTES)return 0;
    if(tileInput.available()){offlineMap_cancelTile();return -1;}
    tileInput.close();completed=tileBuffer;tileBuffer=nullptr;tileOffset=0;return 1;
}
uint16_t *offlineMap_readTile(OfflineMapStyle style,uint8_t zoom,uint32_t x,uint32_t y){
    if(!offlineMap_startTile(style,zoom,x,y))return nullptr;
    uint16_t *result=nullptr;
    while(offlineMap_stepTile(result)==0){}
    return result;
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
    return lakeStorage_ready() && offlineMap_checkManifest(root);
}
OfflineMapStyle offlineMap_getStyle(){return g_style;}
