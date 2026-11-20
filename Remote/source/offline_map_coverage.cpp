#include "offline_map_coverage.h"
#include "lake_storage.h"
#include <SD.h>
#include <math.h>
#include <esp_heap_caps.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

static constexpr uint16_t INDEX_LIMIT = 6000;
static constexpr uint16_t CHECK_TILE_BUDGET = 6000;
struct __attribute__((packed)) CoverageRecord { uint8_t z; uint32_t x,y; uint8_t present; };
static CoverageRecord *rows = nullptr;
static uint16_t rowCount = 0, presentCount = 0, missingCount = 0;
static bool valid = false;
static bool calculationCapped = false;

static int compareKey(const CoverageRecord &r, uint8_t z, uint32_t x, uint32_t y)
{
    if (r.z != z) return r.z < z ? -1 : 1;
    if (r.x != x) return r.x < x ? -1 : 1;
    if (r.y != y) return r.y < y ? -1 : 1;
    return 0;
}

static bool parseUnsignedField(const char *text, unsigned long maxValue, unsigned long &out)
{
    if (!text || !*text) return false;
    for (const char *p = text; *p; ++p) if (*p < '0' || *p > '9') return false;
    errno = 0;
    char *end = nullptr;
    const unsigned long value = strtoul(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || value > maxValue) return false;
    out = value;
    return true;
}

bool mapCoverage_begin()
{
    if (rows) { heap_caps_free(rows); rows = nullptr; }
    rowCount = presentCount = missingCount = 0;
    valid = false;
    calculationCapped = false;

    File f = SD.open(lakeStorage_path("/maps/coverage_index.csv").c_str(), FILE_READ);
    if (!f) return false;
    rows = (CoverageRecord *)heap_caps_malloc(sizeof(CoverageRecord) * INDEX_LIMIT, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!rows) { f.close(); return false; }

    char line[48];
    bool header = false;
    int previousZ = -1;
    uint32_t previousX = 0, previousY = 0;
    while (f.available())
    {
        size_t n = f.readBytesUntil('\n', line, sizeof(line) - 1);
        line[n] = '\0';
        if (n == sizeof(line) - 1 && f.available())
        {
            f.close(); rowCount = 0; heap_caps_free(rows); rows = nullptr;
            return false; // Reject a line too long for the parser rather than accepting a truncated record.
        }
        if (n && line[n - 1] == '\r') line[--n] = '\0';
        if (!n) continue;
        if (!header)
        {
            header = true;
            if (strcmp(line, "z,x,y,present") != 0) { f.close(); heap_caps_free(rows); rows = nullptr; return false; }
            continue;
        }
        if (rowCount >= INDEX_LIMIT) { f.close(); rowCount = 0; heap_caps_free(rows); rows = nullptr; return false; }

        char *fields[4] = {nullptr, nullptr, nullptr, nullptr};
        char *cursor = line;
        for (uint8_t i = 0; i < 4; ++i)
        {
            fields[i] = cursor;
            char *comma = strchr(cursor, ',');
            if (i < 3)
            {
                if (!comma) { f.close(); heap_caps_free(rows); rows = nullptr; rowCount = 0; return false; }
                *comma = '\0'; cursor = comma + 1;
            }
            else if (comma) { f.close(); heap_caps_free(rows); rows = nullptr; rowCount = 0; return false; }
        }
        unsigned long zValue, xValue, yValue, pValue;
        if (!parseUnsignedField(fields[0], 22, zValue) ||
            !parseUnsignedField(fields[1], UINT32_MAX, xValue) ||
            !parseUnsignedField(fields[2], UINT32_MAX, yValue) ||
            !parseUnsignedField(fields[3], 1, pValue))
        {
            f.close(); heap_caps_free(rows); rows = nullptr; rowCount = 0; return false;
        }
        const uint32_t nTiles = 1UL << zValue;
        if (xValue >= nTiles || yValue >= nTiles ||
            (rowCount && ((int)zValue < previousZ ||
             ((int)zValue == previousZ && (xValue < previousX ||
              (xValue == previousX && yValue <= previousY)))))
        {
            f.close(); heap_caps_free(rows); rows = nullptr; rowCount = 0; return false;
        }
        previousZ = (int)zValue; previousX = (uint32_t)xValue; previousY = (uint32_t)yValue;
        rows[rowCount++] = {(uint8_t)zValue, (uint32_t)xValue, (uint32_t)yValue, (uint8_t)pValue};
        if (pValue) ++presentCount; else ++missingCount;
    }
    f.close();
    valid = header && rowCount > 0;
    if (!valid) { heap_caps_free(rows); rows = nullptr; rowCount = presentCount = missingCount = 0; }
    return valid;
}

bool mapCoverage_hasIndex() { return valid; }
bool mapCoverage_tilePresent(uint8_t z, uint32_t x, uint32_t y)
{
    if (!valid || z > 22) return false;
    int lo = 0, hi = (int)rowCount - 1;
    while (lo <= hi)
    {
        const int mid = lo + (hi - lo) / 2;
        const int cmp = compareKey(rows[mid], z, x, y);
        if (!cmp) return rows[mid].present != 0;
        if (cmp < 0) lo = mid + 1; else hi = mid - 1;
    }
    return false;
}
void mapCoverage_getStats(uint16_t &planned, uint16_t &ok, uint16_t &missing)
{
    planned = rowCount; ok = presentCount; missing = missingCount;
}

static constexpr double MERCATOR_LIMIT = 85.05112878;
static int tileX(double lon, uint8_t z)
{
    const double wrapped = fmod(lon + 180.0, 360.0);
    const double normalized = (wrapped < 0.0 ? wrapped + 360.0 : wrapped) / 360.0;
    return (int)floor(normalized * (1UL << z));
}
static int tileY(double lat, uint8_t z)
{
    lat = fmax(-MERCATOR_LIMIT, fmin(MERCATOR_LIMIT, lat));
    const double r = lat * M_PI / 180.0;
    return (int)floor((1.0 - asinh(tan(r)) / M_PI) * 0.5 * (1UL << z));
}

bool mapCoverage_calculationCapped() { return calculationCapped; }

void mapCoverage_checkSquare(double lat, double lon, uint16_t sideM, const uint8_t *zooms,
                             uint8_t count, uint16_t &needed, uint16_t &covered, uint16_t &miss)
{
    needed = covered = miss = 0;
    calculationCapped = false;
    if (!valid || !zooms || !count || !isfinite(lat) || !isfinite(lon) || lat < -90.0 || lat > 90.0)
        return;
    const double half = sideM / 2.0;
    const double dLat = half / 111320.0;
    const double cosLat = fabs(cos(lat * M_PI / 180.0));
    if (cosLat < 1e-5) return;
    const double dLon = half / (111320.0 * cosLat);
    const double west = lon - dLon, east = lon + dLon;
    const double north = fmin(MERCATOR_LIMIT, lat + dLat);
    const double south = fmax(-MERCATOR_LIMIT, lat - dLat);

    uint32_t processed = 0;
    for (uint8_t k = 0; k < count; ++k)
    {
        const uint8_t z = zooms[k];
        if (z > 22) continue;
        const int n = 1 << z;
        const int x0 = tileX(west, z), x1 = tileX(east, z);
        int y0 = tileY(north, z), y1 = tileY(south, z);
        if (y0 > y1) { const int t = y0; y0 = y1; y1 = t; }
        y0 = (y0 < 0) ? 0 : y0;
        y1 = (y1 >= n) ? n - 1 : y1;
        const uint32_t xSpan = (east - west >= 360.0) ? (uint32_t)n : (uint32_t)((x1 - x0 + n) % n + 1);
        const uint32_t ySpan = y1 >= y0 ? (uint32_t)(y1 - y0 + 1) : 0U;
        const uint64_t levelCount = (uint64_t)xSpan * ySpan;
        if (levelCount > CHECK_TILE_BUDGET - processed)
        {
            calculationCapped = true;
            needed = CHECK_TILE_BUDGET; covered = 0; miss = CHECK_TILE_BUDGET;
            return; // Bound work on extreme latitude/zoom selections.
        }
        for (uint32_t dx = 0; dx < xSpan; ++dx)
        {
            const uint32_t wx = (uint32_t)(((int64_t)x0 + dx + n) % n);
            for (uint32_t y = (uint32_t)y0; y <= (uint32_t)y1; ++y)
            {
                ++processed; ++needed;
                if (mapCoverage_tilePresent(z, wx, y)) ++covered; else ++miss;
            }
        }
    }
}
