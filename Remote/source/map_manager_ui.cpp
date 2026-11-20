#include "map_manager_ui.h"
#include "offline_map_coverage.h"
#include "offline_map.h"
#include "map_renderer.h"
#include "boat_state.h"
#include <SD.h>
#include <esp_heap_caps.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static constexpr uint16_t LIMIT=6000;struct ImportRow{uint8_t z;uint32_t x,y;};
static lv_obj_t *page,*centreLabel,*estimateLabel,*packLabel,*coverageLabel,*progress,*importLabel,*importBtn;static lv_obj_t*zoomLabels[6];static float lat=43,lon=25;static bool chosen[6]={true,true,true,true,false,false};static uint8_t zlist[6]={14,15,16,17,18,19};static MapManagerReturnCallback onBack;
static ImportRow *importRows=nullptr;static uint16_t importCount=0,importIndex=0;static bool importing=false;static uint8_t importStyle=0;static File inFile,outFile;static uint8_t copyPhase=0;static const char*copyFiles[2]={"/map-import/coverage_index.csv","/map-import/manifest.txt"};static uint32_t copied=0,totalBytes=0;
static lv_obj_t*label(lv_obj_t*p,const char*t,int x,int y,int w=750){lv_obj_t*l=lv_label_create(p);lv_label_set_text(l,t);lv_obj_set_pos(l,x,y);lv_obj_set_width(l,w);lv_label_set_long_mode(l,LV_LABEL_LONG_WRAP);return l;}
static void refresh()
{
    char text[220];
    snprintf(text, sizeof(text), "ЦЕНТЪР %.6f, %.6f | КВАДРАТ 1 км", lat, lon);
    lv_label_set_text(centreLabel, text);

    uint8_t selectedZooms[6], selectedCount = 0;
    uint64_t tileEstimate = 0;
    const double safeLat = fmax(-85.0, fmin(85.0, (double)lat));
    const double dLat = 500.0 / 111320.0;
    const double cosLat = fmax(0.0871557, fabs(cos(safeLat * M_PI / 180.0)));
    const double dLon = 500.0 / (111320.0 * cosLat);

    for (int i = 0; i < 6; ++i)
    {
        char zoomText[12];
        snprintf(zoomText, sizeof(zoomText), "Z%u%s", (unsigned)zlist[i], chosen[i] ? " *" : "");
        lv_label_set_text(zoomLabels[i], zoomText);
        if (!chosen[i]) continue;
        selectedZooms[selectedCount++] = zlist[i];
        const uint64_t n = 1ULL << zlist[i];
        const double west = lon - dLon, east = lon + dLon;
        const int x0 = (int)floor((fmod(west + 180.0 + 360.0, 360.0)) / 360.0 * n);
        const int x1 = (int)floor((fmod(east + 180.0 + 360.0, 360.0)) / 360.0 * n);
        const double north = fmin(85.05112878, safeLat + dLat);
        const double south = fmax(-85.05112878, safeLat - dLat);
        const int y0 = (int)floor((1.0 - asinh(tan(north * M_PI / 180.0)) / M_PI) * 0.5 * n);
        const int y1 = (int)floor((1.0 - asinh(tan(south * M_PI / 180.0)) / M_PI) * 0.5 * n);
        const uint64_t xSpan = (east - west >= 360.0) ? n : (uint64_t)((x1 - x0 + (int)n) % (int)n + 1);
        const uint64_t ySpan = (uint64_t)(abs(y1 - y0) + 1);
        tileEstimate += xSpan * ySpan;
    }

    const uint64_t estimateBytes = tileEstimate * (uint64_t)OFFLINE_MAP_TILE_BYTES;
    const double mib = estimateBytes / 1048576.0;
    snprintf(text, sizeof(text), "ОЦЕНКА ~%llu плочки | %.2f MiB | %.2f MiB +10%%",
             (unsigned long long)tileEstimate, mib, mib * 1.10);
    lv_label_set_text(estimateLabel, text);

    uint16_t total = 0, present = 0, missing = 0;
    offlineMap_getCoverageStats(total, present, missing);
    snprintf(text, sizeof(text), "ПАКЕТ: %s | %u индексирани / %u налични / %u липсващи",
             offlineMap_hasMapData() ? "VALID" : "NONE", total, present, missing);
    lv_label_set_text(packLabel, text);

    uint16_t needed = 0, covered = 0, absent = 0;
    mapCoverage_checkSquare(safeLat, lon, 1000, selectedZooms, selectedCount, needed, covered, absent);
    if (mapCoverage_calculationCapped())
        snprintf(text, sizeof(text), "ПОКРИТИЕ: ИЗЧИСЛЕНИЕТО Е ОГРАНИЧЕНО (>%u плочки)", (unsigned)needed);
    else
        snprintf(text, sizeof(text), "ПОКРИТИЕ: %u/%u плочки | %u липсват", covered, needed, absent);
    lv_label_set_text(coverageLabel, text);
    lv_bar_set_value(progress, needed ? (int)((uint32_t)covered * 100U / needed) : 0, LV_ANIM_OFF);
}
static void nLat(int d){lat=fmax(-85.0f,fmin(85.0f,lat+d*.001f));refresh();}static void nLon(int d){lon+=d*.001f;if(lon>180)lon-=360;if(lon< -180)lon+=360;refresh();}
static void zoomToggle(lv_event_t*e){int i=(intptr_t)lv_event_get_user_data(e);chosen[i]=!chosen[i];refresh();}
static bool parseManifestValue(const char *manifest, const char *key, char *out, size_t outSize)
{
    if (!manifest || !key || !out || outSize < 2) return false;
    const size_t keyLen = strlen(key);
    bool found = false;
    out[0] = '\0';
    const char *line = manifest;
    while (*line)
    {
        const char *end = strchr(line, '\n');
        const size_t len = end ? (size_t)(end - line) : strlen(line);
        if (len && line[len - 1] == '\r')
        {
            if (len > keyLen && memcmp(line, key, keyLen) == 0 && line[keyLen] == '=')
            {
                if (found || len - keyLen - 1 >= outSize) return false;
                memcpy(out, line + keyLen + 1, len - keyLen - 1);
                out[len - keyLen - 1] = '\0';
                found = true;
            }
        }
        else if (len > keyLen && memcmp(line, key, keyLen) == 0 && line[keyLen] == '=')
        {
            if (found || len - keyLen - 1 >= outSize) return false;
            memcpy(out, line + keyLen + 1, len - keyLen - 1);
            out[len - keyLen - 1] = '\0';
            found = true;
        }
        if (!end) break;
        line = end + 1;
    }
    return found && out[0] != '\0';
}
static bool parseStrictUnsigned(const char *text, unsigned long max, unsigned long &value)
{
    if (!text || !*text) return false;
    for (const char *p = text; *p; ++p) if (*p < '0' || *p > '9') return false;
    char *end = nullptr;
    const unsigned long parsed = strtoul(text, &end, 10);
    if (end == text || *end || parsed > max) return false;
    value = parsed;
    return true;
}
static bool manifestHasLine(const char *manifest, const char *required)
{
    if (!manifest || !required) return false;
    const size_t reqLen = strlen(required);
    const char *line = manifest;
    while (*line)
    {
        const char *end = strchr(line, '\n');
        const size_t len = end ? (size_t)(end - line) : strlen(line);
        if (len && line[len - 1] == '\r')
        {
            if (len - 1 == reqLen && memcmp(line, required, reqLen) == 0) return true;
        }
        else if (len == reqLen && memcmp(line, required, reqLen) == 0) return true;
        if (!end) break;
        line = end + 1;
    }
    return false;
}

static bool copyStart(const char*src,const char*dst){inFile=SD.open(src,FILE_READ);if(!inFile)return false;if(strstr(src,".rgb565")&&inFile.size()!=OFFLINE_MAP_TILE_BYTES){inFile.close();return false;}char dir[100];strncpy(dir,dst,sizeof(dir)-1);dir[sizeof(dir)-1]=0;char*last=strrchr(dir,'/');if(last){*last=0;SD.mkdir(dir);}SD.remove(dst);outFile=SD.open(dst,FILE_WRITE);if(!outFile){inFile.close();return false;}return true;}
// Return -1 on I/O error, 0 while copying, and 1 when the file is complete.
static int copyChunk(){if(!inFile||!outFile)return -1;uint8_t buf[4096];size_t n=inFile.read(buf,sizeof(buf));if(n){if(outFile.write(buf,n)!=n){inFile.close();outFile.close();return -1;}copied+=n;}if(!inFile.available()){inFile.close();outFile.close();return 1;}return 0;}
static void importStart(lv_event_t *event)
{
    (void)event;
    if (importing || !SD.exists("/map-import/manifest.txt") || !SD.exists("/map-import/coverage_index.csv"))
    {
        lv_label_set_text(importLabel, "No staged pack at /map-import/");
        return;
    }
    File manifestFile = SD.open("/map-import/manifest.txt", FILE_READ);
    if (!manifestFile || manifestFile.size() >= 512)
    {
        if (manifestFile) manifestFile.close();
        lv_label_set_text(importLabel, "Manifest missing or exceeds 511-byte validation limit.");
        return;
    }
    char manifest[512] = {};
    const size_t manifestSize = manifestFile.size();
    const size_t manifestRead = manifestFile.readBytes(manifest, manifestSize);
    manifestFile.close();
    if (manifestRead != manifestSize)
    {
        lv_label_set_text(importLabel, "Manifest read failed; pack rejected.");
        return;
    }

    char value[128];
    unsigned long expected = 0;
    if (!manifestHasLine(manifest, "pack_complete=true") ||
        !parseManifestValue(manifest, "attribution", value, sizeof(value)) ||
        !parseManifestValue(manifest, "licence_url", value, sizeof(value)) ||
        !parseManifestValue(manifest, "zoom_levels", value, sizeof(value)) ||
        !parseManifestValue(manifest, "exported_tile_count", value, sizeof(value)) ||
        !parseStrictUnsigned(value, LIMIT, expected) || expected == 0)
    {
        lv_label_set_text(importLabel, "Staged manifest invalid, incomplete, or missing metadata.");
        return;
    }
    char styleValue[32];
    if (!parseManifestValue(manifest, "style", styleValue, sizeof(styleValue)))
    {
        lv_label_set_text(importLabel, "Manifest style is missing or duplicated.");
        return;
    }
    if (strcmp(styleValue, "street") == 0) importStyle = 0;
    else if (strcmp(styleValue, "satellite") == 0) importStyle = 1;
    else
    {
        lv_label_set_text(importLabel, "Staged pack must declare style=street or satellite.");
        return;
    }

    if (importRows) { heap_caps_free(importRows); importRows = nullptr; }
    importRows = (ImportRow *)heap_caps_malloc(sizeof(ImportRow) * LIMIT, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!importRows)
    {
        lv_label_set_text(importLabel, "PSRAM allocation failed.");
        return;
    }

    File indexFile = SD.open("/map-import/coverage_index.csv", FILE_READ);
    if (!indexFile)
    {
        heap_caps_free(importRows); importRows = nullptr;
        lv_label_set_text(importLabel, "Coverage index could not be opened.");
        return;
    }
    char line[48];
    bool header = false, indexBad = false;
    importCount = 0;
    int previousZ = -1;
    uint32_t previousX = 0, previousY = 0;
    while (indexFile.available())
    {
        size_t n = indexFile.readBytesUntil('\n', line, sizeof(line) - 1);
        line[n] = '\0';
        if (n == sizeof(line) - 1 && indexFile.available()) { indexBad = true; break; }
        if (n && line[n - 1] == '\r') line[--n] = '\0';
        if (!n) continue;
        if (!header)
        {
            header = true;
            if (strcmp(line, "z,x,y,present") != 0) indexBad = true;
            if (indexBad) break;
            continue;
        }
        if (importCount >= LIMIT) { indexBad = true; break; }
        char *fields[4] = {nullptr, nullptr, nullptr, nullptr};
        char *cursor = line;
        for (uint8_t i = 0; i < 4; ++i)
        {
            fields[i] = cursor;
            char *comma = strchr(cursor, ',');
            if (i < 3)
            {
                if (!comma) { indexBad = true; break; }
                *comma = '\0'; cursor = comma + 1;
            }
            else if (comma) indexBad = true;
        }
        if (indexBad) break;
        unsigned long z, x, y, present;
        if (!parseStrictUnsigned(fields[0], 22, z) ||
            !parseStrictUnsigned(fields[1], UINT32_MAX, x) ||
            !parseStrictUnsigned(fields[2], UINT32_MAX, y) ||
            !parseStrictUnsigned(fields[3], 1, present) || present != 1 ||
            x >= (1UL << z) || y >= (1UL << z))
        {
            indexBad = true; break;
        }
        if (importCount && ((int)z < previousZ ||
            ((int)z == previousZ && (x < previousX || (x == previousX && y <= previousY))))
        {
            indexBad = true; break;
        }
        previousZ = (int)z; previousX = (uint32_t)x; previousY = (uint32_t)y;
        importRows[importCount++] = {(uint8_t)z, (uint32_t)x, (uint32_t)y};
    }
    indexFile.close();
    if (!header || indexBad || !importCount || importCount != expected)
    {
        heap_caps_free(importRows); importRows = nullptr; importCount = 0;
        lv_label_set_text(importLabel, "Coverage index malformed, unsorted, duplicate, or tile-count mismatch.");
        return;
    }

    File indexSizeFile = SD.open("/map-import/coverage_index.csv", FILE_READ);
    const uint32_t indexBytes = indexSizeFile ? indexSizeFile.size() : 0;
    if (indexSizeFile) indexSizeFile.close();
    File manifestSizeFile = SD.open("/map-import/manifest.txt", FILE_READ);
    const uint32_t manifestBytes = manifestSizeFile ? manifestSizeFile.size() : 0;
    if (manifestSizeFile) manifestSizeFile.close();
    if (!indexBytes || !manifestBytes)
    {
        heap_caps_free(importRows); importRows = nullptr; importCount = 0;
        lv_label_set_text(importLabel, "Could not determine staged pack sizes.");
        return;
    }

    importIndex = 0; copyPhase = 0; copied = 0;
    totalBytes = (uint32_t)((uint64_t)importCount * OFFLINE_MAP_TILE_BYTES + indexBytes + manifestBytes);
    // Invalidate the active pack before touching its tile files. A partial copy
    // must never be accepted as a valid installed pack.
    SD.remove("/maps/manifest.txt");
    importing = true;
    lv_label_set_text(importLabel, "ИМПОРТ: копиране към активната папка...");
}
static void back(lv_event_t*e){if(lv_event_get_code(e)==LV_EVENT_CLICKED&&!importing){lv_obj_del(page);if(onBack)onBack();}}
static void button(lv_obj_t*p,const char*t,int x,int y,int w,lv_event_cb_t cb,void*ud=nullptr){lv_obj_t*b=lv_button_create(p);lv_obj_set_size(b,w,38);lv_obj_set_pos(b,x,y);lv_obj_set_style_bg_color(b,lv_color_hex(0x174F92),LV_PART_MAIN);lv_obj_set_style_radius(b,8,LV_PART_MAIN);lv_obj_set_style_border_width(b,1,LV_PART_MAIN);lv_obj_set_style_border_color(b,lv_color_hex(0x347FC0),LV_PART_MAIN);lv_obj_set_style_text_color(b,lv_color_hex(0xFFFFFF),LV_PART_MAIN);lv_label_set_text(lv_label_create(b),t);lv_obj_center(lv_obj_get_child(b,0));lv_obj_add_event_cb(b,cb,LV_EVENT_CLICKED,ud);}
void mapManagerUI_open(lv_obj_t*parent,float gpsLat,float gpsLon,bool gpsValid,MapManagerReturnCallback cb){onBack=cb;lat=gpsValid?gpsLat:43;lon=gpsValid?gpsLon:25;page=lv_obj_create(parent);lv_obj_set_size(page,790,440);lv_obj_center(page);lv_obj_set_style_bg_color(page,lv_color_hex(0x081522),LV_PART_MAIN);lv_obj_set_style_pad_all(page,10,LV_PART_MAIN);lv_obj_set_style_radius(page,10,LV_PART_MAIN);lv_obj_set_style_border_width(page,1,LV_PART_MAIN);lv_obj_set_style_border_color(page,lv_color_hex(0x2366A4),LV_PART_MAIN);label(page,"МЕНИДЖЪР НА ОФЛАЙН КАРТИ",8,4,400);centreLabel=label(page,"",8,38);label(page,"ПРОМЯНА НА ЦЕНТЪРА ±0.001° (~111 м)",8,70);button(page,"ШИР +",8,100,80,[](lv_event_t*){nLat(1);});button(page,"ШИР -",94,100,80,[](lv_event_t*){nLat(-1);});button(page,"ДЪЛ -",180,100,80,[](lv_event_t*){nLon(-1);});button(page,"ДЪЛ +",266,100,80,[](lv_event_t*){nLon(1);});button(page,"ТЕКУЩ GPS",354,100,130,[](lv_event_t*){if(boatState.gpsValid){lat=boatState.latitude;lon=boatState.longitude;refresh();}});label(page,"МАЩАБ (избери)",8,151);for(int i=0;i<6;i++){lv_obj_t*b=lv_button_create(page);lv_obj_set_size(b,72,40);lv_obj_set_pos(b,8+i*80,176);lv_obj_set_style_bg_color(b,lv_color_hex(0x174F92),LV_PART_MAIN);lv_obj_set_style_radius(b,8,LV_PART_MAIN);lv_obj_set_style_border_width(b,1,LV_PART_MAIN);lv_obj_set_style_border_color(b,lv_color_hex(0x347FC0),LV_PART_MAIN);lv_obj_set_style_text_color(b,lv_color_hex(0xFFFFFF),LV_PART_MAIN);zoomLabels[i]=lv_label_create(b);lv_obj_center(zoomLabels[i]);lv_obj_add_event_cb(b,zoomToggle,LV_EVENT_CLICKED,(void*)(intptr_t)i);}estimateLabel=label(page,"ОЦЕНКА",8,230);packLabel=label(page,"СТАТУС НА ПАКЕТА",8,267);coverageLabel=label(page,"ПОКРИТИЕ",8,304);progress=lv_bar_create(page);lv_obj_set_size(progress,760,20);lv_obj_set_pos(progress,8,337);lv_bar_set_range(progress,0,100);importLabel=label(page,"ИМПОРТ: постави лицензиран пакет в /map-import/",8,361);importBtn=lv_button_create(page);lv_obj_set_size(importBtn,180,38);lv_obj_set_pos(importBtn,8,390);lv_label_set_text(lv_label_create(importBtn),"ИМПОРТ НА ПАКЕТ");lv_obj_center(lv_obj_get_child(importBtn,0));lv_obj_add_event_cb(importBtn,importStart,LV_EVENT_CLICKED,nullptr);button(page,"НАЗАД",665,390,100,back);refresh();}
void mapManagerUI_tick(){static uint32_t lastTick=0;if(!importing||millis()-lastTick<25)return;lastTick=millis();char src[100],dst[100];if(importIndex<importCount){ImportRow r=importRows[importIndex];const char*style=importStyle?"satellite":"street";snprintf(src,sizeof(src),"/map-import/%s/%u/%lu/%lu.rgb565",style,r.z,(unsigned long)r.x,(unsigned long)r.y);snprintf(dst,sizeof(dst),"/maps/%s/%u/%lu/%lu.rgb565",style,r.z,(unsigned long)r.x,(unsigned long)r.y);if(!inFile){if(!copyStart(src,dst)){importing=false;lv_label_set_text(importLabel,"ГРЕШКА: липсва плочка или SD запис/отвор");return;}}int state=copyChunk();if(state<0){importing=false;lv_label_set_text(importLabel,"ГРЕШКА SD — активният пакет остава изключен");return;}if(state==1)importIndex++;}else{if(copyPhase<2){const char*srcPath=copyFiles[copyPhase];const char*dstPath=copyPhase==0?"/maps/coverage_index.csv":"/maps/manifest.txt";if(!inFile){if(!copyStart(srcPath,dstPath)){importing=false;lv_label_set_text(importLabel,"ГРЕШКА при копиране на индекс/manifest");return;}}int state=copyChunk();if(state<0){importing=false;lv_label_set_text(importLabel,"ГРЕШКА индекс/manifest — пакетът е изключен");return;}if(state==1)copyPhase++;}else{importing=false;boatState.offlineMapReady=offlineMap_begin(true);mapRenderer_invalidateTiles();lv_label_set_text(importLabel,boatState.offlineMapReady?"ИМПОРТЪТ ЗАВЪРШИ — ПАКЕТЪТ Е ПРОВЕРЕН":"ИМПОРТЪТ ЗАВЪРШИ, НО ПРОВЕРКАТА НЕ УСПЯ");refresh();}}
char status[100];snprintf(status,sizeof(status),"ИМПОРТ %u/%u плочки | %lu/%lu байта",(unsigned)importIndex,(unsigned)importCount,(unsigned long)copied,(unsigned long)totalBytes);lv_label_set_text(importLabel,status);lv_bar_set_value(progress,totalBytes?copied*100/totalBytes:0,LV_ANIM_OFF);}
