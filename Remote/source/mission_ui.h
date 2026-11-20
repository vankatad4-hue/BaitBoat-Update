#ifndef MISSION_UI_H
#define MISSION_UI_H
#include <lvgl.h>
// Open the waypoint/mission planning screen. This revision stores points in RAM only.
void missionUI_open(lv_obj_t *returnScreen);
void missionUI_initStorage();
bool missionUI_getWaypoint(uint8_t index,float *lat,float *lon,bool *bait,bool *hook);
bool missionUI_getHome(float *lat,float *lon);
#endif
