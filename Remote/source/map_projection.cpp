#include "map_projection.h"
#include <math.h>

static constexpr double MAP_PI = 3.14159265358979323846; // Mathematical pi for projection calculations.
static constexpr double MAP_MAX_LAT = 85.05112878;       // Web-Mercator cannot represent the poles.
static constexpr double MAP_TILE_SIZE = 256.0;           // Standard slippy-map tile width and height.

MapWorldPixel mapProjection_toWorldPixel(double latitude, double longitude, uint8_t zoom)
{
    // Keep shifts and world-size calculations within the supported XYZ zoom range.
    if (zoom > 22)
        return {0.0, 0.0};

    const double lat = fmax(-MAP_MAX_LAT, fmin(MAP_MAX_LAT, latitude)); // Clamp latitude to Mercator limits.
    const double lon = fmax(-180.0, fmin(180.0, longitude));            // Clamp longitude to valid WGS84 range.
    const double worldSize = MAP_TILE_SIZE * (double)(1UL << zoom);     // Calculate world width in pixels.
    const double sinLat = sin(lat * MAP_PI / 180.0);                    // Convert latitude to radians and calculate sine.
    MapWorldPixel result;                                               // Prepare the projected coordinate.
    result.x = (lon + 180.0) / 360.0 * worldSize;                        // Project longitude to horizontal world pixels.
    result.y = (0.5 - log((1.0 + sinLat) / (1.0 - sinLat)) / (4.0 * MAP_PI)) * worldSize; // Project latitude to vertical pixels.
    return result;                                                      // Return the projected point.
}

void mapProjection_toLatLon(const MapWorldPixel &pixel, uint8_t zoom, double &latitude, double &longitude)
{
    // Reject unsupported zoom levels instead of performing an undefined shift.
    if (zoom > 22)
    {
        latitude = 0.0;
        longitude = 0.0;
        return;
    }

    const double worldSize = MAP_TILE_SIZE * (double)(1UL << zoom);     // Recreate world size at this zoom.
    const double x = fmax(0.0, fmin(worldSize, pixel.x));               // Keep inverse projection within map bounds.
    const double y = fmax(0.0, fmin(worldSize, pixel.y));               // Keep inverse projection within map bounds.
    longitude = x / worldSize * 360.0 - 180.0;                          // Convert horizontal pixels to longitude.
    const double n = MAP_PI - 2.0 * MAP_PI * y / worldSize;             // Convert vertical pixels to Mercator ordinate.
    latitude = 180.0 / MAP_PI * atan(0.5 * (exp(n) - exp(-n)));          // Convert Mercator ordinate to latitude.
}

MapWorldPixel mapProjection_screenOffsetToWorld(double screenX, double screenY, double headingDeg)
{
    // A non-finite heading must not contaminate every projected viewport point.
    if (!isfinite(headingDeg))
        return {screenX, screenY};

    const double angle = headingDeg * MAP_PI / 180.0;                   // Convert compass heading to radians.
    const double c = cos(angle);                                        // Cache cosine for the rotation.
    const double s = sin(angle);                                        // Cache sine for the rotation.
    MapWorldPixel result;                                               // Prepare the rotated offset.
    result.x = screenX * c - screenY * s;                               // Rotate horizontal screen offset into map east/west pixels.
    result.y = screenX * s + screenY * c;                               // Rotate vertical screen offset into map north/south pixels.
    return result;                                                      // Return rotated map-world offset.
}

bool mapProjection_pan(double &latitude,double &longitude,uint8_t zoom,double dx,double dy){
 if(zoom>22||!isfinite(latitude)||!isfinite(longitude)||!isfinite(dx)||!isfinite(dy))return false;
 MapWorldPixel p=mapProjection_toWorldPixel(latitude,longitude,zoom);
 const double width=256.0*(1UL<<zoom);
 p.x=fmod(p.x+dx,width);if(p.x<0)p.x+=width;
 p.y=fmax(0.0,fmin(width,p.y+dy));
 mapProjection_toLatLon(p,zoom,latitude,longitude);return true;
}
