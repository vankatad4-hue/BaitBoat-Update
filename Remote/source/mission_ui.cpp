#include "mission_ui.h"
#include "boat_state.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

// This UI deliberately does not send mission or release commands.
// Local waypoint storage is volatile until persistent storage and boat-side
// mission validation/execution are implemented.
static constexpr uint8_t SLOT_COUNT = 4;
struct WaypointSlot { bool valid; float lat, lon; bool baitMark, hookMark; };
static WaypointSlot slots[SLOT_COUNT] = {};
static bool homeValid = false;
static float homeLat = 0.0f, homeLon = 0.0f;
static int selectedSlot = -1;
static Preferences missionPrefs;
static bool storageReady = false;
static constexpr uint32_t STORE_MAGIC = 0x42544D50UL;
struct StoredMission { uint32_t magic; WaypointSlot points[SLOT_COUNT]; bool homeValid; float homeLat, homeLon; int8_t selected; uint32_t crc; };
static uint32_t crc32(const uint8_t *data, size_t len) {
 uint32_t crc=0xFFFFFFFFUL;
 for(size_t i=0;i<len;i++){crc^=data[i];for(uint8_t b=0;b<8;b++)crc=(crc&1)?(crc>>1)^0xEDB88320UL:crc>>1;}
 return ~crc;
}
static bool validCoordinate(float lat,float lon){return isfinite(lat)&&isfinite(lon)&&lat>=-90.0f&&lat<=90.0f&&lon>=-180.0f&&lon<=180.0f;}
static void persist() {
 if(!storageReady)return;
 StoredMission s={};s.magic=STORE_MAGIC;memcpy(s.points,slots,sizeof(slots));s.homeValid=homeValid;s.homeLat=homeLat;s.homeLon=homeLon;s.selected=(int8_t)selectedSlot;
 s.crc=crc32(reinterpret_cast<const uint8_t*>(&s),offsetof(StoredMission,crc));
 missionPrefs.putBytes("mission",&s,sizeof(s));
}
static void loadPersisted() {
 storageReady=missionPrefs.begin("baitmission",false);
 if(!storageReady){setStatus("NVS OPEN FAILED - RAM ONLY");return;}
 StoredMission s={};
 if(missionPrefs.getBytesLength("mission")!=sizeof(s)||missionPrefs.getBytes("mission",&s,sizeof(s))!=sizeof(s)||s.magic!=STORE_MAGIC||s.crc!=crc32(reinterpret_cast<const uint8_t*>(&s),offsetof(StoredMission,crc))){
  memset(slots,0,sizeof(slots));homeValid=false;selectedSlot=-1;setStatus("NVS EMPTY/INVALID - NEW PLAN");return;
 }
 for(uint8_t i=0;i<SLOT_COUNT;i++)if(s.points[i].valid&&!validCoordinate(s.points[i].lat,s.points[i].lon))s.points[i]={};
 memcpy(slots,s.points,sizeof(slots));homeValid=s.homeValid&&validCoordinate(s.homeLat,s.homeLon);
 homeLat=homeValid?s.homeLat:0;homeLon=homeValid?s.homeLon:0;
 selectedSlot=(s.selected>=0&&s.selected<SLOT_COUNT&&slots[(uint8_t)s.selected].valid)?s.selected:-1;
 setStatus("PLAN LOADED FROM NVS");
}

static lv_obj_t *screen = nullptr;
static lv_obj_t *returnScreen = nullptr;
static lv_obj_t *gpsLabel = nullptr;
static lv_obj_t *homeLabel = nullptr;
static lv_obj_t *slotLabels[SLOT_COUNT] = {};
static lv_obj_t *statusLabel = nullptr;
static lv_obj_t *selectedLabel = nullptr;
static lv_timer_t *refreshTimer = nullptr;

static lv_color_t bg() { return lv_color_hex(0x101820); }
static lv_color_t panel() { return lv_color_hex(0x1C2935); }
static lv_color_t blue() { return lv_color_hex(0x1565C0); }
static lv_color_t green() { return lv_color_hex(0x2E7D32); }
static lv_color_t red() { return lv_color_hex(0xC62828); }

static lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y, int w)
{
    lv_obj_t *o=lv_label_create(parent); lv_label_set_text(o,text);
    lv_obj_set_pos(o,x,y); lv_obj_set_width(o,w);
    lv_obj_set_style_text_color(o,lv_color_hex(0xFFFFFF),LV_PART_MAIN);
    lv_label_set_long_mode(o,LV_LABEL_LONG_WRAP); return o;
}
static lv_obj_t *button(lv_obj_t *parent,const char *txt,int x,int y,int w,int h,lv_color_t color)
{
    lv_obj_t *b=lv_button_create(parent); lv_obj_set_size(b,w,h); lv_obj_set_pos(b,x,y);
    lv_obj_set_style_bg_color(b,color,LV_PART_MAIN);
    lv_obj_t *t=lv_label_create(b); lv_label_set_text(t,txt); lv_obj_center(t); return b;
}
static bool currentFixIsFresh()
{
    return boatState.gpsValid && (uint32_t)(millis()-boatState.gpsLastUpdateMs)<=2000U;
}
static void refresh()
{
    if(!screen) return;
    char buf[150];
    if(currentFixIsFresh()) snprintf(buf,sizeof(buf),"CURRENT FIX: %.6f, %.6f  |  age %lu ms",boatState.latitude,boatState.longitude,(unsigned long)(millis()-boatState.gpsLastUpdateMs));
    else snprintf(buf,sizeof(buf),"CURRENT FIX: INVALID / STALE — SAVE DISABLED");
    lv_label_set_text(gpsLabel,buf);
    if(homeValid) snprintf(buf,sizeof(buf),"HOME (RAM): %.6f, %.6f",homeLat,homeLon);
    else snprintf(buf,sizeof(buf),"HOME: NOT SET");
    lv_label_set_text(homeLabel,buf);
    for(uint8_t i=0;i<SLOT_COUNT;i++){
        if(slots[i].valid) snprintf(buf,sizeof(buf),"WP%u  %.6f, %.6f  %s%s",(unsigned)(i+1),slots[i].lat,slots[i].lon,slots[i].baitMark?"BAIT MARK ":"",slots[i].hookMark?"HOOK MARK":"");
        else snprintf(buf,sizeof(buf),"WP%u  EMPTY",(unsigned)(i+1));
        lv_label_set_text(slotLabels[i],buf);
    }
    if(selectedSlot>=0 && slots[selectedSlot].valid) snprintf(buf,sizeof(buf),"SELECTED: WP%d",(selectedSlot+1));
    else snprintf(buf,sizeof(buf),"SELECTED: NONE");
    lv_label_set_text(selectedLabel,buf);
}
static void setStatus(const char *s){ if(statusLabel) lv_label_set_text(statusLabel,s); }
static void selectSlot(lv_event_t *e)
{
    if(lv_event_get_code(e)!=LV_EVENT_CLICKED) return;
    int index=(int)(intptr_t)lv_event_get_user_data(e);
    if(index<0||index>=SLOT_COUNT||!slots[index].valid){setStatus("SLOT EMPTY — SAVE A CURRENT FIX FIRST");return;}
    selectedSlot=index; refresh(); setStatus("WAYPOINT SELECTED (RAM ONLY)");
}
static void saveCurrent(lv_event_t *e)
{
    if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;
    if(!currentFixIsFresh()){setStatus("SAVE BLOCKED: NEED FRESH VALID GNSS FIX");return;}
    int slot=-1;
    if(selectedSlot>=0&&!slots[selectedSlot].valid) slot=selectedSlot;
    if(slot<0){for(int i=0;i<SLOT_COUNT;i++)if(!slots[i].valid){slot=i;break;}}
    if(slot<0){setStatus("ALL FOUR SLOTS FULL — SELECT AND CLEAR ONE");return;}
    slots[slot]={true,boatState.latitude,boatState.longitude,false,false};
    selectedSlot=slot;refresh();setStatus("WAYPOINT SAVED IN RAM — LOST ON POWER OFF");
}
static void setHome(lv_event_t *e)
{
    if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;
    if(!currentFixIsFresh()){setStatus("HOME SET BLOCKED: NEED FRESH VALID GNSS FIX");return;}
    homeLat=boatState.latitude;homeLon=boatState.longitude;homeValid=true;
    persist();refresh();setStatus(storageReady?"HOME SAVED TO NVS":"HOME SAVED RAM ONLY");
}
static void clearSlot(lv_event_t *e)
{
    if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;
    if(selectedSlot<0||!slots[selectedSlot].valid){setStatus("SELECT A VALID WAYPOINT TO CLEAR");return;}
    slots[selectedSlot]={}; selectedSlot=-1;persist();refresh();setStatus("WAYPOINT CLEARED");
}
static void toggleBait(lv_event_t *e)
{
    if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;
    if(selectedSlot<0||!slots[selectedSlot].valid){setStatus("SELECT A WAYPOINT FIRST");return;}
    slots[selectedSlot].baitMark=!slots[selectedSlot].baitMark;persist();refresh();
    setStatus("BAIT RELEASE MARKER EDITED — PLANNING ONLY, NOT A COMMAND");
}
static void toggleHook(lv_event_t *e)
{
    if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;
    if(selectedSlot<0||!slots[selectedSlot].valid){setStatus("SELECT A WAYPOINT FIRST");return;}
    slots[selectedSlot].hookMark=!slots[selectedSlot].hookMark;persist();refresh();
    setStatus("HOOK RELEASE MARKER EDITED — PLANNING ONLY, NOT A COMMAND");
}
static void startMission(e)
{
    if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;
    setStatus("MISSION START DISABLED: PERSISTENCE, BOAT EXECUTOR, AUTH/ACK AND SAFETY GATES NOT READY");
}
static void back(e)
{
    if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;
    if(refreshTimer){lv_timer_delete(refreshTimer);refreshTimer=nullptr;}
    if(returnScreen)lv_screen_load(returnScreen);
    screen=nullptr;
}
static void timerRefresh(lv_timer_t *t){(void)t;refresh();}

void missionUI_open(lv_obj_t *main)
{
    returnScreen=main;
    loadPersisted();
    screen=lv_obj_create(nullptr);lv_obj_set_style_bg_color(screen,bg(),LV_PART_MAIN);
    lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);
    label(screen,"WAYPOINTS / MISSION PLAN — LOCAL ONLY",15,10,760);
    gpsLabel=label(screen,"CURRENT FIX: CHECKING",15,42,760);
    homeLabel=label(screen,"HOME: NOT SET",15,72,760);
    lv_obj_t *p=lv_obj_create(screen);lv_obj_set_pos(p,10,105);lv_obj_set_size(p,780,220);
    lv_obj_set_style_bg_color(p,panel(),LV_PART_MAIN);lv_obj_set_style_pad_all(p,8,LV_PART_MAIN);
    label(p,"SAVED WAYPOINTS (NVS)",8,6,750);
    const int ys[4]={38,78,118,158};
    const int colors[4]={0x1565C0,0x1565C0,0x1565C0,0x1565C0};
    for(int i=0;i<4;i++){
        slotLabels[i]=label(p,"WP EMPTY",8,ys[i],510);
        lv_obj_t *b=button(p,"SELECT",530,ys[i]-5,110,34,lv_color_hex(colors[i]));
        lv_obj_add_event_cb(b,selectSlot,LV_EVENT_CLICKED,(void*)(intptr_t)i);
    }
    selectedLabel=label(screen,"SELECTED: NONE",15,333,760);
    lv_obj_t *save=button(screen,"SAVE CURRENT",15,360,150,45,green());
    lv_obj_add_event_cb(save,saveCurrent,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *setH=button(screen,"SET HOME",175,360,130,45,blue());
    lv_obj_add_event_cb(setH,setHome,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *clear=button(screen,"CLEAR SLOT",315,360,130,45,red());
    lv_obj_add_event_cb(clear,clearSlot,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *bait=button(screen,"BAIT MARK",455,360,130,45,blue());
    lv_obj_add_event_cb(bait,toggleBait,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *hook=button(screen,"HOOK MARK",595,360,130,45,blue());
    lv_obj_add_event_cb(hook,toggleHook,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *start=button(screen,"START MISSION",15,410,180,45,red());
    lv_obj_add_event_cb(start,startMission,LV_EVENT_CLICKED,nullptr);
    lv_obj_t *backBtn=button(screen,"BACK",650,410,130,45,blue());
    lv_obj_add_event_cb(backBtn,back,LV_EVENT_CLICKED,nullptr);
    statusLabel=label(screen,"MISSION EXECUTION DISABLED — UI PLANNING ONLY",205,418,430);
    refresh();lv_screen_load(screen);
    refreshTimer=lv_timer_create(timerRefresh,250,nullptr);
}
