#include "mission_ui.h"
#include "boat_state.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <Preferences.h>
#include <SD.h>
#include <math.h>
#include <stddef.h>

static constexpr uint8_t SLOT_COUNT = 16;
struct WaypointSlot { bool valid; float lat, lon; bool baitMark, hookMark; };
static WaypointSlot slots[SLOT_COUNT] = {};
static bool homeValid=false; static float homeLat=0,homeLon=0; static int selectedSlot=-1;
static void setStatus(const char *s);
static Preferences missionPrefs; static bool storageReady=false,storageAttempted=false;
static constexpr uint32_t STORE_MAGIC=0x42544D50UL;
struct StoredMission { uint32_t magic; WaypointSlot points[SLOT_COUNT]; bool homeValid; float homeLat,homeLon; int8_t selected; uint32_t crc; };
// ver026 storage record used four slots; migrate it when its CRC validates.
struct LegacyStoredMission { uint32_t magic; WaypointSlot points[4]; bool homeValid; float homeLat,homeLon; int8_t selected; uint32_t crc; };
static uint32_t crc32(const uint8_t *data,size_t len){uint32_t crc=0xFFFFFFFFUL;for(size_t i=0;i<len;i++){crc^=data[i];for(uint8_t b=0;b<8;b++)crc=(crc&1)?(crc>>1)^0xEDB88320UL:crc>>1;}return ~crc;}
static bool validCoordinate(float lat,float lon){return isfinite(lat)&&isfinite(lon)&&lat>=-90&&lat<=90&&lon>=-180&&lon<=180;}
static bool sdMissionAvailable(){return SD.cardType()!=CARD_NONE;}
static bool writeSdMission(const StoredMission &record){
 if(!sdMissionAvailable())return false;
 if(!SD.exists("/missions")&&!SD.mkdir("/missions"))return false;
 const char *tmp="/missions/waypoints.tmp",*dst="/missions/waypoints.bin";
 if(SD.exists(tmp))SD.remove(tmp);
 File out=SD.open(tmp,FILE_WRITE);if(!out)return false;
 size_t written=out.write(reinterpret_cast<const uint8_t*>(&record),sizeof(record));out.flush();out.close();
 if(written!=sizeof(record)){SD.remove(tmp);return false;}
 File verify=SD.open(tmp,FILE_READ);StoredMission check={};bool ok=verify&&verify.read(reinterpret_cast<uint8_t*>(&check),sizeof(check))==sizeof(check);if(verify)verify.close();
 ok=ok&&check.magic==STORE_MAGIC&&check.crc==crc32(reinterpret_cast<const uint8_t*>(&check),offsetof(StoredMission,crc));
 if(!ok){SD.remove(tmp);return false;}
 if(SD.exists(dst))SD.remove(dst);
 if(!SD.rename(tmp,dst)){SD.remove(tmp);return false;}
 return true;
}
static bool readSdMission(StoredMission &record){
 if(!sdMissionAvailable()||!SD.exists("/missions/waypoints.bin"))return false;
 File in=SD.open("/missions/waypoints.bin",FILE_READ);if(!in)return false;
 bool ok=in.size()==sizeof(record)&&in.read(reinterpret_cast<uint8_t*>(&record),sizeof(record))==sizeof(record);in.close();
 return ok&&record.magic==STORE_MAGIC&&record.crc==crc32(reinterpret_cast<const uint8_t*>(&record),offsetof(StoredMission,crc));
}
static bool persist(){
 StoredMission r={};r.magic=STORE_MAGIC;memcpy(r.points,slots,sizeof(slots));r.homeValid=homeValid;r.homeLat=homeLat;r.homeLon=homeLon;r.selected=(int8_t)selectedSlot;r.crc=crc32(reinterpret_cast<const uint8_t*>(&r),offsetof(StoredMission,crc));
 bool sdPresent=sdMissionAvailable();bool sdOk=sdPresent?writeSdMission(r):false;
 bool nvsOk=storageReady&&missionPrefs.putBytes("mission",&r,sizeof(r))==sizeof(r);
 return sdPresent?sdOk:(nvsOk);
}
static void applyStored(const StoredMission &r){
 for(uint8_t i=0;i<SLOT_COUNT;i++)if(r.points[i].valid&&!validCoordinate(r.points[i].lat,r.points[i].lon))slots[i]={};
 memcpy(slots,r.points,sizeof(slots));for(uint8_t i=0;i<SLOT_COUNT;i++)if(slots[i].valid&&!validCoordinate(slots[i].lat,slots[i].lon))slots[i]={};
 homeValid=r.homeValid&&validCoordinate(r.homeLat,r.homeLon);homeLat=homeValid?r.homeLat:0;homeLon=homeValid?r.homeLon:0;
 selectedSlot=(r.selected>=0&&r.selected<SLOT_COUNT&&slots[(uint8_t)r.selected].valid)?r.selected:-1;
}
static void loadPersisted(){
 if(storageAttempted)return;storageAttempted=true;storageReady=missionPrefs.begin("baitmission",false);
 StoredMission sdRecord={};
 if(readSdMission(sdRecord)){applyStored(sdRecord);if(storageReady)missionPrefs.putBytes("mission",&sdRecord,sizeof(sdRecord));setStatus("16 ТОЧКИ ВЪЗСТАНОВЕНИ ОТ SD");return;}
 if(!storageReady){setStatus("NVS ГРЕШКА — НЕ Е НАМЕРЕН SD ЗАПИС");return;}
 StoredMission s={};size_t len=missionPrefs.getBytesLength("mission");
 if(len==sizeof(s)&&missionPrefs.getBytes("mission",&s,sizeof(s))==sizeof(s)&&s.magic==STORE_MAGIC&&s.crc==crc32(reinterpret_cast<const uint8_t*>(&s),offsetof(StoredMission,crc))){
  applyStored(s);bool migrated=sdMissionAvailable()?writeSdMission(s):true;setStatus(migrated?"ПЛАНЪТ Е ВЪЗСТАНОВЕН / SD":"ПЛАНЪТ Е ВЪЗСТАНОВЕН / SD ГРЕШКА");return;
 }
 LegacyStoredMission legacy={};
 if(len==sizeof(legacy)&&missionPrefs.getBytes("mission",&legacy,sizeof(legacy))==sizeof(legacy)&&legacy.magic==STORE_MAGIC&&legacy.crc==crc32(reinterpret_cast<const uint8_t*>(&legacy),offsetof(LegacyStoredMission,crc))){
  for(uint8_t i=0;i<4;i++)if(legacy.points[i].valid&&validCoordinate(legacy.points[i].lat,legacy.points[i].lon))slots[i]=legacy.points[i];
  homeValid=legacy.homeValid&&validCoordinate(legacy.homeLat,legacy.homeLon);homeLat=homeValid?legacy.homeLat:0;homeLon=homeValid?legacy.homeLon:0;
  selectedSlot=(legacy.selected>=0&&legacy.selected<4&&slots[(uint8_t)legacy.selected].valid)?legacy.selected:-1;
  bool upgraded=persist();setStatus(upgraded?"VER026 МИГРИРАН / SD ФАЙЛ":"СТАР ПЛАН / ГРЕШКА ПРИ МИГРАЦИЯ");return;
 }
 memset(slots,0,sizeof(slots));homeValid=false;selectedSlot=-1;setStatus("ПРАЗНО / НЕВАЛИДНО ХРАНИЛИЩЕ");
}
static lv_obj_t *screen=nullptr,*returnScreen=nullptr,*gpsLabel=nullptr,*homeLabel=nullptr,*slotLabels[SLOT_COUNT]={},*statusLabel=nullptr,*selectedLabel=nullptr;static lv_timer_t *refreshTimer=nullptr;
static lv_color_t bg(){return lv_color_hex(0x081522);}static lv_color_t panel(){return lv_color_hex(0x10243A);}static lv_color_t blue(){return lv_color_hex(0x174F92);}static lv_color_t green(){return lv_color_hex(0x20C979);}static lv_color_t red(){return lv_color_hex(0xD91E2B);}
static lv_obj_t *label(lv_obj_t *p,const char *t,int x,int y,int w){lv_obj_t *o=lv_label_create(p);lv_label_set_text(o,t);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,w);lv_obj_set_style_text_color(o,lv_color_hex(0xFFFFFF),LV_PART_MAIN);lv_label_set_long_mode(o,LV_LABEL_LONG_WRAP);return o;}
static lv_obj_t *button(lv_obj_t *p,const char *t,int x,int y,int w,int h,lv_color_t c){lv_obj_t *b=lv_button_create(p);lv_obj_set_size(b,w,h);lv_obj_set_pos(b,x,y);lv_obj_set_style_bg_color(b,c,LV_PART_MAIN);lv_obj_set_style_radius(b,8,LV_PART_MAIN);lv_obj_set_style_border_width(b,1,LV_PART_MAIN);lv_obj_set_style_border_color(b,lv_color_hex(0x347FC0),LV_PART_MAIN);lv_obj_set_style_text_color(b,lv_color_hex(0xFFFFFF),LV_PART_MAIN);lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,t);lv_obj_center(l);return b;}
static bool currentFixIsFresh(){return boatState.gpsValid&&(uint32_t)(millis()-boatState.gpsLastUpdateMs)<=2000U;}
static void refresh(){if(!screen)return;char b[160];if(currentFixIsFresh())snprintf(b,sizeof(b),"GPS: %.6f, %.6f | възраст %lu ms",boatState.latitude,boatState.longitude,(unsigned long)(millis()-boatState.gpsLastUpdateMs));else snprintf(b,sizeof(b),"GPS: НЕВАЛИДЕН / СТАР — ЗАПИСЪТ Е БЛОКИРАН");lv_label_set_text(gpsLabel,b);if(homeValid)snprintf(b,sizeof(b),"ДОМ: %.6f, %.6f",homeLat,homeLon);else snprintf(b,sizeof(b),"ДОМ: НЕ Е ЗАДАДЕН");lv_label_set_text(homeLabel,b);for(uint8_t i=0;i<SLOT_COUNT;i++){if(slots[i].valid)snprintf(b,sizeof(b),"Т%02u %s%s",(unsigned)(i+1),slots[i].baitMark?"B ":"",slots[i].hookMark?"H":"");else snprintf(b,sizeof(b),"Т%02u ПРАЗЕН",(unsigned)(i+1));lv_label_set_text(slotLabels[i],b);}if(selectedSlot>=0&&slots[selectedSlot].valid)snprintf(b,sizeof(b),"ИЗБРАН Т%02d",(selectedSlot+1));else snprintf(b,sizeof(b),"ИЗБРАН: НЯМА");lv_label_set_text(selectedLabel,b);}
static void setStatus(const char *s){if(statusLabel)lv_label_set_text(statusLabel,s);}
static void selectSlot(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;int i=(int)(intptr_t)lv_event_get_user_data(e);if(i<0||i>=SLOT_COUNT||!slots[i].valid){setStatus("ПРАЗНО — ЗАПИШИ АКТУАЛЕН GPS");return;}selectedSlot=i;bool ok=persist();refresh();setStatus(ok?"ТОЧКАТА Е ИЗБРАНА / SD ЗАПИСАНА":"ГРЕШКА ПРИ SD ЗАПИС");}
static void saveCurrent(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(!currentFixIsFresh()){setStatus("БЛОКИРАНО: НУЖЕН Е GPS ПОД 2 СЕК.");return;}int i=(selectedSlot>=0)?selectedSlot:-1;if(i<0||slots[i].valid){i=-1;for(int n=0;n<SLOT_COUNT;n++)if(!slots[n].valid){i=n;break;}}if(i<0){setStatus("16 ТОЧКИ СА ЗАПЪЛНЕНИ — ИЗТРИЙ ИЛИ ИЗБЕР");return;}slots[i]={true,boatState.latitude,boatState.longitude,false,false};selectedSlot=i;bool ok=persist();refresh();setStatus(ok?"ТОЧКАТА Е ЗАПИСАНА НА SD":"SD ГРЕШКА — ЗАПИСЪТ НЕ Е ПОТВЪРДЕН");}
static void setHome(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(!currentFixIsFresh()){setStatus("ДОМ: НУЖЕН Е АКТУАЛЕН GPS");return;}homeLat=boatState.latitude;homeLon=boatState.longitude;homeValid=true;bool ok=persist();refresh();setStatus(ok?"ДОМЪТ Е ЗАПИСАН НА SD":"SD WRITE FAILED");}
static void clearSlot(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(selectedSlot<0||!slots[selectedSlot].valid){setStatus("ИЗБЕРИ ТОЧКА ЗА ИЗТРИВАНЕ");return;}slots[selectedSlot]={};selectedSlot=-1;bool ok=persist();refresh();setStatus(ok?"ТОЧКАТА Е ИЗТРИТА / SD ЗАПИСАНА":"SD WRITE FAILED");}
static void toggleFlag(lv_event_t *e,bool bait){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(selectedSlot<0||!slots[selectedSlot].valid){setStatus("ПЪРВО ИЗБЕРИ ТОЧКА");return;}if(bait)slots[selectedSlot].baitMark=!slots[selectedSlot].baitMark;else slots[selectedSlot].hookMark=!slots[selectedSlot].hookMark;bool ok=persist();refresh();setStatus(ok?"МАРКЕРЪТ Е ЗАПИСАН — САМО ПЛАНИРАНЕ":"SD WRITE FAILED");}
static void toggleBait(lv_event_t *e){toggleFlag(e,true);}static void toggleHook(lv_event_t *e){toggleFlag(e,false);}
static void startMission(lv_event_t *e){if(lv_event_get_code(e)==LV_EVENT_CLICKED)setStatus("СТАРТЪТ Е БЛОКИРАН — СИГУРНОСТТА НЕ Е ГОТОВА");}
static void back(lv_event_t *e){if(lv_event_get_code(e)!=LV_EVENT_CLICKED)return;if(refreshTimer){lv_timer_delete(refreshTimer);refreshTimer=nullptr;}if(returnScreen)lv_screen_load(returnScreen);screen=nullptr;}
static void timerRefresh(lv_timer_t *t){(void)t;refresh();}
void missionUI_open(lv_obj_t *main){returnScreen=main;loadPersisted();screen=lv_obj_create(nullptr);lv_obj_set_style_bg_color(screen,bg(),LV_PART_MAIN);lv_obj_remove_flag(screen,LV_OBJ_FLAG_SCROLLABLE);label(screen,"НАВИГАЦИОННИ ТОЧКИ — 16 МЕСТА",12,8,770);gpsLabel=label(screen,"ПРОВЕРКА НА GPS",12,35,770);homeLabel=label(screen,"ДОМ: НЕ Е ЗАДАДЕН",12,60,770);
 lv_obj_t *p=lv_obj_create(screen);lv_obj_set_pos(p,8,88);lv_obj_set_size(p,784,246);lv_obj_set_style_bg_color(p,panel(),LV_PART_MAIN);lv_obj_set_style_pad_all(p,5,LV_PART_MAIN);label(p,"ИЗБЕРИ ТОЧКА — SD / NVS РЕЗЕРВ",4,3,760);
 for(uint8_t i=0;i<SLOT_COUNT;i++){int col=i%4,row=i/4;int x=col*190+5,y=row*50+25;slotLabels[i]=label(p,"Т00 ПРАЗЕН",x,y,118);lv_obj_t *s=button(p,"ИЗБ.",x+122,y-5,54,34,blue());lv_obj_add_event_cb(s,selectSlot,LV_EVENT_CLICKED,(void*)(intptr_t)i);}
 selectedLabel=label(screen,"ИЗБРАН: НЯМА",12,342,770);lv_obj_t *save=button(screen,"ЗАПИС GPS",10,366,130,42,green());lv_obj_add_event_cb(save,saveCurrent,LV_EVENT_CLICKED,nullptr);lv_obj_t *h=button(screen,"ЗАДАЙ ДОМ",148,366,125,42,blue());lv_obj_add_event_cb(h,setHome,LV_EVENT_CLICKED,nullptr);lv_obj_t *c=button(screen,"ИЗТРИЙ",281,366,100,42,red());lv_obj_add_event_cb(c,clearSlot,LV_EVENT_CLICKED,nullptr);lv_obj_t *b=button(screen,"ЗАХРАНКА",389,366,100,42,blue());lv_obj_add_event_cb(b,toggleBait,LV_EVENT_CLICKED,nullptr);lv_obj_t *k=button(screen,"КУКА",497,366,100,42,blue());lv_obj_add_event_cb(k,toggleHook,LV_EVENT_CLICKED,nullptr);lv_obj_t *start=button(screen,"СТАРТ",605,366,90,42,red());lv_obj_add_event_cb(start,startMission,LV_EVENT_CLICKED,nullptr);lv_obj_t *bb=button(screen,"НАЗАД",700,366,90,42,blue());lv_obj_add_event_cb(bb,back,LV_EVENT_CLICKED,nullptr);statusLabel=label(screen,"ИЗПЪЛНЕНИЕТО НА МИСИЯ Е ИЗКЛЮЧЕНО",12,418,770);refresh();lv_screen_load(screen);refreshTimer=lv_timer_create(timerRefresh,250,nullptr);}
void missionUI_initStorage(){if(!storageReady)loadPersisted();}
bool missionUI_getWaypoint(uint8_t i,float *lat,float *lon,bool *bait,bool *hook){if(i>=SLOT_COUNT||!slots[i].valid)return false;if(lat)*lat=slots[i].lat;if(lon)*lon=slots[i].lon;if(bait)*bait=slots[i].baitMark;if(hook)*hook=slots[i].hookMark;return true;}
bool missionUI_getHome(float *lat,float *lon){if(!homeValid)return false;if(lat)*lat=homeLat;if(lon)*lon=homeLon;return true;}
int8_t missionUI_getSelectedIndex(){return (selectedSlot>=0&&selectedSlot<SLOT_COUNT&&slots[selectedSlot].valid)?(int8_t)selectedSlot:-1;}
