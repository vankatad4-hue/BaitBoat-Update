#ifndef MAP_PROJECTION_H
#define MAP_PROJECTION_H

#include <Arduino.h>

// A Web-Mercator world-pixel coordinate at a selected slippy-map zoom level.
struct MapWorldPixel
{
    double x; // Horizontal world pixel, increasing east.
    double y; // Vertical world pixel, increasing south.
};

// Convert WGS84 latitude/longitude to Web-Mercator world pixels.
// Latitude is clamped to the supported Web-Mercator limit (±85.05112878°).
MapWorldPixel mapProjection_toWorldPixel(double latitude, double longitude, uint8_t zoom);

// Convert Web-Mercator world pixels back to WGS84 coordinates.
void mapProjection_toLatLon(const MapWorldPixel &pixel, uint8_t zoom, double &latitude, double &longitude);

// Rotate a screen-relative pixel offset into map-world pixel offset.
// headingDeg is compass heading clockwise from true north; zero means north-up.
MapWorldPixel mapProjection_screenOffsetToWorld(double screenX, double screenY, double headingDeg);

bool mapProjection_pan(double &latitude,double &longitude,uint8_t zoom,double dx,double dy);
#endif
