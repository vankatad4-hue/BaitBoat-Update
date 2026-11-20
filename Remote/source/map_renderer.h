#ifndef MAP_RENDERER_H
#define MAP_RENDERER_H
#include <lvgl.h>
#include <stdint.h>
#include "offline_map.h"
struct MapRendererWaypoint { bool valid; float lat,lon; bool bait,hook; };
void mapRenderer_create(lv_obj_t *viewport);
void mapRenderer_update(bool gpsValid,float boatLat,float boatLon,float heading,
                        bool homeValid,float homeLat,float homeLon,
                        const MapRendererWaypoint *waypoints,uint8_t selectedIndex,
                        bool returnHome);
void mapRenderer_zoomIn();
void mapRenderer_zoomOut();
uint8_t mapRenderer_getZoom();
void mapRenderer_setStyle(OfflineMapStyle style);
void mapRenderer_invalidateTiles();
void mapRenderer_resetProfile();
#endif
