#include "lake_ui.h"
#include "lake_storage.h"
#include "mission_ui.h"
#include "offline_map.h"
#include "map_renderer.h"
#include "boat_state.h"
#include "boat_ui.h"
void boatUI_refreshLakeLabel();
static lv_obj_t *page=nullptr,*parentScreen=nullptr,*status=nullptr,*field=nullptr,*keyboard=nullptr,*labels[LAKE_COUNT]={};
static uint8_t selected=0;
static lv_obj_t *label(lv_obj_t *p,const char *text,int x,int y,int w){
    lv_obj_t *l=lv_label_create(p);lv_label_set_text(l,text);lv_obj_set_pos(l,x,y);lv_obj_set_width(l,w);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);return l;
}
static lv_obj_t *button(lv_obj_t *p,const char *text,int x,int y,int w,int h,lv_event_cb_t cb,void *ud=nullptr){
    lv_obj_t *b=lv_button_create(p);lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,h);
    lv_obj_set_style_bg_color(b,lv_color_hex(0x174F92),LV_PART_MAIN);
    lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,text);lv_obj_center(l);
    lv_obj_add_event_cb(b,cb,LV_EVENT_CLICKED,ud);return b;
}
static void refreshNames(){
    for(uint8_t i=0;i<LAKE_COUNT;i++){
        char name[LAKE_NAME_BYTES],line[80];
        lakeStorage_getName(i,name,sizeof(name));
        snprintf(line,sizeof(line),"%02u %s%s%s",(unsigned)i+1,i==lakeStorage_active()?"* ":"",i==selected?"> ":"",name);
        lv_label_set_text(labels[i],line);
    }
}
static void pick(lv_event_t *e){
    selected=(uint8_t)(intptr_t)lv_event_get_user_data(e);
    char name[LAKE_NAME_BYTES];lakeStorage_getName(selected,name,sizeof(name));
    lv_textarea_set_text(field,name);refreshNames();
    lv_label_set_text(status,"ИЗБЕРИ АКТИВИРАНЕ ИЛИ ПРОМЕНИ ИМЕ");
}
static void activate(lv_event_t *){
    if(!lakeStorage_select(selected)){lv_label_set_text(status,"БЛОКИРАНО: SD, РАДИО, РЪЧЕН РЕЖИМ И ПОТВЪРДЕН STOP");return;}
    missionUI_reloadStorage();
    boatState.offlineMapReady=offlineMap_begin(boatState.sdCardReady);
    mapRenderer_resetProfile();
    if(boatState.offlineMapReady)mapRenderer_setStyle(offlineMap_getStyle());
    updateBoatScreen(boatState);refreshNames();
    lv_label_set_text(status,"ЕЗЕРОТО Е АКТИВНО — STOP ОСТАВА ВКЛЮЧЕН");
}
static void saveName(lv_event_t *){
    bool ok=lakeStorage_setName(selected,lv_textarea_get_text(field));refreshNames();
    lv_label_set_text(status,ok?"ИМЕТО Е ЗАПИСАНО НА SD":"ГРЕШКА SD / НЕВАЛИДНО ИМЕ");
}
static void showKeyboard(lv_event_t *){lv_keyboard_set_textarea(keyboard,field);lv_obj_remove_flag(keyboard,LV_OBJ_FLAG_HIDDEN);lv_obj_move_foreground(keyboard);}
static void hideKeyboard(lv_event_t *){lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);}
bool lakeUI_close(){if(!page||lv_screen_active()!=page)return false;
    if(keyboard&&!lv_obj_has_flag(keyboard,LV_OBJ_FLAG_HIDDEN)){lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);return true;}
    lv_obj_t *old=page;page=nullptr;boatUI_refreshLakeLabel();lv_screen_load(parentScreen);lv_obj_delete_async(old);return true;
}
static void closePage(lv_event_t *){lakeUI_close();}
void lakeUI_open(lv_obj_t *parent){
    if(page)return;
    parentScreen=parent;selected=lakeStorage_active();
    page=lv_obj_create(nullptr);lv_obj_set_style_bg_color(page,lv_color_hex(0x081522),LV_PART_MAIN);
    lv_obj_set_style_text_color(page,lv_color_hex(0xFFFFFF),LV_PART_MAIN);lv_obj_remove_flag(page,LV_OBJ_FLAG_SCROLLABLE);
    label(page,"ЕЗЕРА — 8 ПРОФИЛА / 16 ТОЧКИ ВСЕКИ",12,10,770);
    for(uint8_t i=0;i<LAKE_COUNT;i++){
        int x=12+(i%2)*390,y=42+(i/2)*56;
        lv_obj_t *b=button(page,"",x,y,376,48,pick,(void*)(intptr_t)i);
        labels[i]=lv_obj_get_child(b,0);lv_obj_set_width(labels[i],350);lv_label_set_long_mode(labels[i],LV_LABEL_LONG_DOT);
    }
    label(page,"ИМЕ НА ИЗБРАНОТО ЕЗЕРО",12,277,760);
    field=lv_textarea_create(page);lv_obj_set_pos(field,12,302);lv_obj_set_size(field,555,48);
    lv_textarea_set_one_line(field,true);lv_textarea_set_max_length(field,LAKE_NAME_BYTES-1);
    lv_obj_add_event_cb(field,showKeyboard,LV_EVENT_CLICKED,nullptr);
    button(page,"ЗАПАЗИ ИМЕ",580,302,208,48,saveName);
    button(page,"АКТИВИРАЙ",12,369,240,48,activate);
    button(page,"НАЗАД",548,369,240,48,closePage);
    status=label(page,"* АКТИВНО | > ИЗБРАНО | СМЯНА САМО ПРИ STOP",12,432,775);
    keyboard=lv_keyboard_create(page);lv_obj_set_size(keyboard,800,205);lv_obj_align(keyboard,LV_ALIGN_BOTTOM_MID,0,0);
    lv_keyboard_set_mode(keyboard,LV_KEYBOARD_MODE_TEXT_LOWER);lv_keyboard_set_textarea(keyboard,field);
    lv_obj_add_event_cb(keyboard,hideKeyboard,LV_EVENT_READY,nullptr);lv_obj_add_event_cb(keyboard,hideKeyboard,LV_EVENT_CANCEL,nullptr);
    lv_obj_add_flag(keyboard,LV_OBJ_FLAG_HIDDEN);
    char name[LAKE_NAME_BYTES];lakeStorage_getName(selected,name,sizeof(name));lv_textarea_set_text(field,name);
    refreshNames();lv_screen_load(page);
}
