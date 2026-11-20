#ifndef MISSION_UI_H
#define MISSION_UI_H
#include <lvgl.h>
#include <stdint.h>
static constexpr uint8_t MISSION_WAYPOINT_CAPACITY = 16;
void missionUI_open(lv_obj_t *returnScreen);
void missionUI_initStorage();
bool missionUI_getWaypoint(uint8_t index,float *lat,float *lon,bool *bait,bool *hook);
bool missionUI_getHome(float *lat,float *lon);
int8_t missionUI_getSelectedIndex();
void missionUI_reloadStorage();
void missionUI_saveCurrent();
void missionUI_selectNext();
bool missionUI_close();
bool missionUI_isOpen();
#endif
