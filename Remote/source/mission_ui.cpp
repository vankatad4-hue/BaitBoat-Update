#include "mission_ui.h"
#include "mission_storage.h"
#include "boat_state.h"
#include "lake_storage.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <SD.h>
#include <math.h>
#include <stddef.h>

static WaypointSlot slots[SLOT_COUNT] = {};
static bool homeValid=false; static float homeLat=0,homeLon=0; static int selectedSlot=-1;
static void setStatus(const char *s);
static bool storageReady=false,storageAttempted=false;
static bool sdMissionAvailable(){return lakeStorage_ready();}
static void applyStored(const StoredMission &r);
static bool persist(){
 StoredMission r={};r.magic=STORE_MAGIC;memcpy(r.points,slots,sizeof(slots));r.homeValid=homeValid;r.homeLat=homeLat;r.homeLon=homeLon;r.selected=(int8_t)selectedSlot;r.crc=missionStorage_crc32(reinterpret_cast<const uint8_t*>(&r),offsetof(StoredMission,crc));
 if(missionStorage_write(r))return true;
 StoredMission old={}; if(missionStorage_read(old)){applyStored(old);}else{memset(slots,0,sizeof(slots));homeValid=false;selectedSlot=-1;storageReady=false;}return false;
}
static void applyStored(const StoredMission &r){
 StoredMission safe=r;missionStorage_sanitize(safe);
 memcpy(slots,safe.points,sizeof(slots));homeValid=safe.homeValid;homeLat=safe.homeLat;homeLon=safe.homeLon;selectedSlot=safe.selected;
}
static void loadPersisted(){
 if(storageAttempted)return;storageAttempted=true;
 memset(slots,0,sizeof(slots));homeValid=false;homeLat=homeLon=0;selectedSlot=-1;
 storageReady=sdMissionAvailable();if(!storageReady)return;
 StoredMission record={};
 if(missionStorage_read(record)){applyStored(record);return;}
 // Never replace a corrupt profile with another lake's data or a legacy record.
 if(SD.exists(lakeStorage_path("/waypoints.bin").c_str())||
    SD.exists(lakeStorage_path("/waypoints.bak").c_str())){storageReady=false;return;}
 if(lakeStorage_active()==0 && missionStorage_readRecord("/missions/waypoints.bin",record)){
  if(missionStorage_write(record))applyStored(record);else storageReady=false;
 }
}
void missionUI_reloadStorage(){storageAttempted=false;loadPersisted();}
static lv_obj_t *screen=nullptr,*returnScreen=nullptr,*gpsLabel=nullptr,*homeLabel=nullptr,*slotLabels[SLOT_COUNT]={},*statusLabel=nullptr,*selectedLabel=nullptr,*targetMetricsLabel=nullptr;static lv_timer_t *refreshTimer=nullptr;
static lv_color_t bg(){return lv_color_hex(0x081522);}static lv_color_t panel(){return lv_color_hex(0x10243A);}static lv_color_t blue(){return lv_color_hex(0x174F92);}static lv_color_t green(){return lv_color_hex(0x20C979);}static lv_color_t red(){return lv_color_hex(0xD91E2B);}
static lv_obj_t *label(lv_obj_t *p,const char *t,int x,int y,int w){lv_obj_t *o=lv_label_create(p);lv_label_set_text(o,t);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);lv_obj_set_style_text_color(o,lv_color_hex(0xFFFFFF),LV_PART_MAIN);lv_label_set_long_mode(o,LV_LABEL_LONG_WRAP);return o;}
static lv_obj_t *button(lv_obj_t *p,const char *t,int x,int y,int w,int h,lv_color_t c){lv_obj_t *b=lv_button_create(p);lv_obj_set_size(b,w,h);lv_obj_set_pos(b,x,y);lv_obj_set_style_bg_color(b,c,LV_PART_MAIN);lv_obj_set_style_radius(b,8,LV_PART_MAIN);lv_obj_set_style_border_width(b,1,LV_PART_MAIN);lv_obj_set_style_border_color(b,lv_color_hex(0x347FC0),LV_PART_MAIN);lv_obj_set_style_text_color(b,lv_color_hex(0xFFFFFF),LV_PART_MAIN);lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,t);lv_obj_center(l);return b;}
static bool currentFixIsFresh(){return boatState.gpsValid&&(uint32_t)(millis()-boatState.gpsLastUpdateMs)<=2000U;}
static void refresh(){
 if(!screen)return;char b[180];
 if(currentFixIsFresh())snprintf(b,sizeof(b),"GPS: %.6f, %.6f | възраст %lu ms",boatState.latitude,boatState.longitude,(unsigned long)(millis()-boatState.gpsLastUpdateMs));
 else snprintf(b,sizeof(b),"GPS: НЕВАЛИДЕН / СТАР — ЗАПИСЪТ Е БЛОКИРАН");
 lv_label_set_text(gpsLabel,b);
 if(homeValid)snprintf(b,sizeof(b),"ДОМ: %.6f, %.6f",homeLat,homeLon);else snprintf(b,sizeof(b),"ДОМ: НЕ Е ЗАДАДЕН");
 lv_label_set_text(homeLabel,b);
 for(uint8_t i=0;i<SLOT_COUNT;i++){if(slots[i].valid)snprintf(b,sizeof(b),"Т%02u %s%s",(unsigned)(i+1),slots[i].baitMark?"З ":"",slots[i].hookMark?"К":"");else snprintf(b,sizeof(b),"Т%02u ПРАЗЕН",(unsigned)(i+1));lv_label_set_text(slotLabels[i],b);}
 if(selectedSlot>=0&&slots[selectedSlot].valid){
  snprintf(b,sizeof(b),"ИЗБРАНА Т%02u: %.6f, %.6f",(selectedSlot+1),slots[selectedSlot].lat,slots[selectedSlot].lon);lv_label_set_text(selectedLabel,b);
  if(currentFixIsFresh()){
   const double rad=0.017453292519943295;double la1=boatState.latitude*rad,la2=slots[selectedSlot].lat*rad,dl=(slots[selectedSlot].lon-boatState.longitude)*rad;
   double dlat=la2-la1;double a=sin(dlat/2)*sin(dlat/2)+cos(la1)*cos(la2)*sin(dl/2)*sin(dl/2);
   a=fmin(1.0,fmax(0.0,a));
   double dist=6371000.0*2.0*atan2(sqrt(a),sqrt(1.0-a));double y=sin(dl)*cos(la2),x=cos(la1)*sin(la2)-sin(la1)*cos(la2)*cos(dl);
   double brg=fmod(atan2(y,x)/rad+360.0,360.0);snprintf(b,sizeof(b),"ДИСТАНЦИЯ: %.0f m | ПОСОКА: %.0f° (GPS)",dist,brg);lv_label_set_text(targetMetricsLabel,b);
  }else lv_label_set_text(targetMetricsLabel,"ДИСТАНЦИЯ / ПОСОКА: НЯМА АКТУАЛЕН GPS");
 }else{lv_label_set_text(selectedLabel,"ИЗБРАНА ТОЧКА: НЯМА");lv_label_set_text(targetMetricsLabel,"ИЗБЕРИ ЗАПИСАНА ТОЧКА ЗА НАВИГАЦИЯ");}
}
static void setStatus(const char *s){if(statusLabel)lv_label_set_text(statusLabel,s);}
static void selectSlot(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(!storageReady||!sdMissionAvailable()){setStatus("SD ГРЕШКА");return;}int i=(int)(intptr_t)lv_event_get_user_data(e);if(i<0||i>=SLOT_COUNT||!slots[i].valid){setStatus("ПРАЗНО — ЗАПИШИ АКТУАЛЕН GPS");return;}selectedSlot=i;bool ok=persist();refresh();setStatus(ok?"ТОЧКАТА Е ИЗБРАНА / SD ЗАПИСАНА":"ГРЕШКА ПРИ SD ЗАПИС");}
static void saveCurrent(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(!storageReady||!sdMissionAvailable()){setStatus("SD ГРЕШКА");return;}if(!currentFixIsFresh()||!missionStorage_validCoordinate(boatState.latitude,boatState.longitude)){setStatus("БЛОКИРАНО: НУЖЕН Е GPS ПОД 2 СЕК.");return;}int i=(selectedSlot>=0)?selectedSlot:-1;if(i<0||slots[i].valid){i=-1;for(int n=0;n<SLOT_COUNT;n++)if(!slots[n].valid){i=n;break;}}if(i<0){setStatus("16 ТОЧКИ СА ЗАПЪЛНЕНИ — ИЗТРИЙ ИЛИ ИЗБЕР");return;}slots[i]={true,boatState.latitude,boatState.longitude,false,false};selectedSlot=i;bool ok=persist();refresh();setStatus(ok?"ТОЧКАТА Е ЗАПИСАНА НА SD":"SD ГРЕШКА — ЗАПИСЪТ НЕ Е ПОТВЪРДЕН");}
static void setHome(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(!storageReady||!sdMissionAvailable()){setStatus("SD ГРЕШКА");return;}if(!currentFixIsFresh()){setStatus("ДОМ: НУЖЕН Е АКТУАЛЕН GPS");return;}homeLat=boatState.latitude;homeLon=boatState.longitude;homeValid=true;bool ok=persist();refresh();setStatus(ok?"ДОМЪТ Е ЗАПИСАН НА SD":"SD WRITE FAILED");}
static void clearSlot(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(!storageReady||!sdMissionAvailable()){setStatus("SD ГРЕШКА");return;}if(selectedSlot<0||!slots[selectedSlot].valid){setStatus("ИЗБЕРИ ТОЧКА ЗА ИЗТРИВАНЕ");return;}slots[selectedSlot]={};selectedSlot=-1;bool ok=persist();refresh();setStatus(ok?"ТОЧКАТА Е ИЗТРИТА / SD ЗАПИСАНА":"SD WRITE FAILED");}
static void toggleFlag(lv_event_t *e,bool bait){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(!storageReady||!sdMissionAvailable()){setStatus("SD ГРЕШКА");return;}if(selectedSlot<0||!slots[selectedSlot].valid){setStatus("ПЪРВО ИЗБЕРИ ТОЧКА");return;}if(bait)slots[selectedSlot].baitMark=!slots[selectedSlot].baitMark;else slots[selectedSlot].hookMark=!slots[selectedSlot].hookMark;bool ok=persist();refresh();setStatus(ok?"МАРКЕРЪТ Е ЗАПИСАН — САМО ПЛАНИРАНЕ":"SD WRITE FAILED");}
static void toggleBait(lv_event_t *e){toggleFlag(e,true);}static void toggleHook(lv_event_t *e){toggleFlag(e,false);}
static void startMission(lv_event_t *e){if(lv_event_get_code(e)==LV_EVENT_CLICKED)setStatus("СТАРТЪТ Е БЛОКИРАН — СИГУРНОСТТА НЕ Е ГОТОВА");}
static void back(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(refreshTimer){lv_timer_delete(refreshTimer);refreshTimer=nullptr;}lv_obj_t *old=screen;screen=nullptr;statusLabel=nullptr;if(returnScreen)lv_screen_load(returnScreen);if(old)lv_obj_delete_async(old);}
static void timerRefresh(lv_timer_t *t){(void)t;refresh();}
void missionUI_open(lv_obj_t *main){if(screen)return;returnScreen=main;loadPersisted();screen=lv_obj_create(nullptr);lv_obj_set_style_bg_color(screen,bg(),LV_PART_MAIN);lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);label(screen,"НАВИГАЦИОННИ ТОЧКИ — 16 МЕСТА",12,8,770);gpsLabel=label(screen,"ПРОВЕРКА НА GPS",12,35,770);homeLabel=label(screen,"ДОМ: НЕ Е ЗАДАДЕН",12,60,770);
 lv_obj_t *p=lv_obj_create(screen);lv_obj_set_pos(p,8,88);lv_obj_set_size(p,784,246);lv_obj_set_style_radius(p,8,LV_PART_MAIN);lv_obj_set_style_bg_color(p,panel(),LV_PART_MAIN);lv_obj_set_style_pad_all(p,5,LV_PART_MAIN);lv_obj_set_style_border_width(p,1,LV_PART_MAIN);lv_obj_set_style_border_color(p,lv_color_hex(0x2366A4),LV_PART_MAIN);label(p,"ИЗБЕРИ ТОЧКА — SD / РЕЗЕРВНО КОПИЕ",4,3,760);
 for(uint8_t i=0;i<SLOT_COUNT;i++){int col=i%4,row=i/4;int x=col*190+5,y=row*50+25;slotLabels[i]=label(p,"Т00 ПРАЗЕН",x,y,118);lv_obj_t *s=button(p,"ИЗБ.",x+122,y-5,54,34,blue());lv_obj_add_event_cb(s,selectSlot,LV_EVENT_CLICKED,(void*)(intptr_t)i);}
 selectedLabel=label(screen,"ИЗБРАНА ТОЧКА: НЯМА",12,337,770);targetMetricsLabel=label(screen,"ИЗБЕРИ ЗАПИСАНА ТОЧКА ЗА НАВИГАЦИЯ",12,358,770);lv_obj_t *save=button(screen,"ЗАПИС GPS",10,382,130,38,green());lv_obj_add_event_cb(save,saveCurrent,LV_EVENT_CLICKED,nullptr);lv_obj_t *h=button(screen,"ЗАДАЙ ДОМ",148,382,125,38,blue());lv_obj_add_event_cb(h,setHome,LV_EVENT_CLICKED,nullptr);lv_obj_t *c=button(screen,"ИЗТРИЙ",281,382,100,38,red());lv_obj_add_event_cb(c,clearSlot,LV_EVENT_CLICKED,nullptr);lv_obj_t *b=button(screen,"ЗАХРАНКА",389,382,100,38,blue());lv_obj_add_event_cb(b,toggleBait,LV_EVENT_CLICKED,nullptr);lv_obj_t *k=button(screen,"КУКА",497,382,100,38,blue());lv_obj_add_event_cb(k,toggleHook,LV_EVENT_CLICKED,nullptr);lv_obj_t *start=button(screen,"СТАРТ",605,382,90,38,red());lv_obj_add_event_cb(start,startMission,LV_EVENT_CLICKED,nullptr);lv_obj_t *bb=button(screen,"НАЗАД",700,382,90,38,blue());lv_obj_add_event_cb(bb,back,LV_EVENT_CLICKED,nullptr);statusLabel=label(screen,"ИЗПЪЛНЕНИЕТО НА МИСИЯ Е ИЗКЛЮЧЕНО",12,430,770);refresh();lv_screen_load(screen);refreshTimer=lv_timer_create(timerRefresh,250,nullptr);}
void missionUI_initStorage(){if(!storageReady)loadPersisted();}
bool missionUI_getWaypoint(uint8_t i,float *lat,float *lon,bool *bait,bool *hook){if(!storageReady||!sdMissionAvailable()||i>=SLOT_COUNT||!slots[i].valid)return false;if(lat)*lat=slots[i].lat;if(lon)*lon=slots[i].lon;if(bait)*bait=slots[i].baitMark;if(hook)*hook=slots[i].hookMark;return true;}
bool missionUI_getHome(float *lat,float *lon){if(!storageReady||!sdMissionAvailable()||!homeValid)return false;if(lat)*lat=homeLat;if(lon)*lon=homeLon;return true;}
int8_t missionUI_getSelectedIndex(){return (storageReady&&sdMissionAvailable()&&selectedSlot>=0&&selectedSlot<SLOT_COUNT&&slots[selectedSlot].valid)?(int8_t)selectedSlot:-1;}
