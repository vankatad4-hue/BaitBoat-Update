#ifndef MAP_MANAGER_UI_H
#define MAP_MANAGER_UI_H
#include <lvgl.h>
typedef void (*MapManagerReturnCallback)();
void mapManagerUI_open(lv_obj_t *parent,float gpsLat,float gpsLon,bool gpsValid,MapManagerReturnCallback callback);
void mapManagerUI_tick();
#endif
