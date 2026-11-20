#include "map_renderer.h"
// LVGL 9.6 exposes cache invalidation in this internal header, not lvgl.h.
#include <misc/cache/instance/lv_image_cache.h>
#include "offline_map.h"
#include "map_projection.h"
#include "lake_storage.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

static constexpr uint8_t TILE_SLOTS=12;
static constexpr uint8_t MIN_ZOOM=5,MAX_ZOOM=19;
struct TileSlot { int32_t x,y; uint8_t z; OfflineMapStyle style; uint16_t *pixels; lv_image_dsc_t dsc; lv_obj_t *image; bool valid; uint32_t retryAt; };
static TileSlot tiles[TILE_SLOTS]={};
static TileSlot *pendingTile=nullptr;
static lv_obj_t *view=nullptr,*boat=nullptr,*home=nullptr,*wps[16]={},*route=nullptr,*zoomText=nullptr,*attributionLabel=nullptr;
static lv_point_precise_t routePoints[2];
static uint8_t zoomLevel=15; static OfflineMapStyle mapStyle=OFFLINE_MAP_STREET;
static double centerLat=0,centerLon=0; static bool haveCenter=false,followBoat=true;
static lv_point_t dragPoint={};static bool dragging=false;
static int viewW=0,viewH=0;
static lv_color_t missingColor(){return lv_color_hex(0x263746);}
static void hideTile(TileSlot &t){if(t.image)lv_obj_add_flag(t.image,LV_OBJ_FLAG_HIDDEN);}
static void releaseTile(TileSlot &t){if(pendingTile==&t){offlineMap_cancelTile();pendingTile=nullptr;}if(t.image&&t.pixels){lv_image_set_src(t.image,nullptr);lv_image_cache_drop(&t.dsc);}if(t.pixels)offlineMap_freeTile(t.pixels);t.pixels=nullptr;t.valid=false;}
// Parent deletion owns the image widgets; detach sources before freeing their buffers.
static void viewportDeleted(lv_event_t *e){
 if(lv_event_get_target_obj(e)!=view)return;
 for(auto &t:tiles){releaseTile(t);t.image=nullptr;}
 view=boat=home=route=zoomText=attributionLabel=nullptr;
 for(auto &w:wps)w=nullptr;
 haveCenter=false;followBoat=true;dragging=false;
}
static void loadTileInto(TileSlot &t,int32_t x,int32_t y,uint8_t z){
 releaseTile(t);t.x=x;t.y=y;t.z=z;t.style=mapStyle;t.valid=true;t.retryAt=millis()+3000U;
 if(offlineMap_startTile(mapStyle,z,(uint32_t)x,(uint32_t)y))pendingTile=&t;
}
static void stepTile(){
 if(!pendingTile)return;
 TileSlot &t=*pendingTile;
 uint16_t *buf=nullptr;
 int result=offlineMap_stepTile(buf);
 if(result==0)return;
 pendingTile=nullptr;t.retryAt=millis()+3000U;
 if(result<0)return;
 t.pixels=buf;
 memset(&t.dsc,0,sizeof(t.dsc));t.dsc.header.magic=LV_IMAGE_HEADER_MAGIC;t.dsc.header.stride=OFFLINE_MAP_TILE_SIZE*2;t.dsc.header.cf=LV_COLOR_FORMAT_RGB565;t.dsc.header.w=OFFLINE_MAP_TILE_SIZE;t.dsc.header.h=OFFLINE_MAP_TILE_SIZE;t.dsc.data_size=OFFLINE_MAP_TILE_BYTES;t.dsc.data=(const uint8_t*)buf;
 lv_image_set_src(t.image,&t.dsc);
}
static void drawTiles()
{
 if(!view)return;
 if(!offlineMap_hasMapData()){for(auto&t:tiles){hideTile(t);releaseTile(t);}return;}
 if(!haveCenter){if(pendingTile)releaseTile(*pendingTile);for(auto&t:tiles)hideTile(t);return;}
 const MapWorldPixel c=mapProjection_toWorldPixel(centerLat,centerLon,zoomLevel);
 const double left=c.x-viewW/2.0,top=c.y-viewH/2.0;
 const int32_t firstX=(int32_t)floor(left/256.0),firstY=(int32_t)floor(top/256.0);
 const int32_t n=(1L<<zoomLevel);
 uint8_t loadsThisUpdate=0;
 // Cancel requests no longer matching the current viewport before doing I/O.
 if(pendingTile){
  const int idx=(int)(pendingTile-tiles),dx=idx%4,dy=idx/4;
  const int32_t tx=firstX+dx,ty=firstY+dy;
  if(ty<0||ty>=n||tx*256.0>=left+viewW||ty*256.0>=top+viewH||pendingTile->x!=(tx%n+n)%n||pendingTile->y!=ty||pendingTile->z!=zoomLevel||pendingTile->style!=mapStyle)releaseTile(*pendingTile);
 }
 if(pendingTile){stepTile();loadsThisUpdate=1;}
 // At most one 4 KiB read or one file-open per renderer update.
 for(int dy=0;dy<3;dy++)for(int dx=0;dx<4;dx++)
 {
  const uint8_t idx=(uint8_t)(dy*4+dx);TileSlot &t=tiles[idx];hideTile(t);
  const int32_t displayX=firstX+dx,ty=firstY+dy;if(ty<0||ty>=n||displayX*256.0>=left+viewW||ty*256.0>=top+viewH){releaseTile(t);continue;}
  const int32_t tileX=(displayX%n+n)%n;
  const bool matches=t.valid&&t.x==tileX&&t.y==ty&&t.z==zoomLevel&&t.style==mapStyle&&(t.pixels||pendingTile==&t||(int32_t)(millis()-t.retryAt)<0);
  if(!matches)
  {
   if(loadsThisUpdate>=1||pendingTile)continue;
   loadTileInto(t,tileX,ty,zoomLevel);++loadsThisUpdate;
  }
  if(!t.pixels)continue;
  lv_obj_set_pos(t.image,(int)round(displayX*256.0-left),(int)round(ty*256.0-top));
  lv_obj_remove_flag(t.image,LV_OBJ_FLAG_HIDDEN);
 }
}
static bool projectPoint(float lat,float lon,int &x,int &y){
 if(!haveCenter)return false;MapWorldPixel c=mapProjection_toWorldPixel(centerLat,centerLon,zoomLevel),p=mapProjection_toWorldPixel(lat,lon,zoomLevel);
 double dx=p.x-c.x,dy=p.y-c.y;if(dx>=(double)(1UL<<zoomLevel)*256/2)dx-=((double)(1UL<<zoomLevel)*256);if(dx< -((double)(1UL<<zoomLevel)*256/2))dx+=((double)(1UL<<zoomLevel)*256);
 x=(int)round(viewW/2.0+dx);y=(int)round(viewH/2.0+dy);return x>=-24&&x<=viewW+24&&y>=-24&&y<=viewH+24;
}
static void panEvent(lv_event_t *e){
 if(!haveCenter)return;
 lv_indev_t *indev=lv_indev_active();if(!indev)return;
 lv_point_t p;lv_indev_get_point(indev,&p);
 if(lv_event_get_code(e)==LV_EVENT_PRESSED){dragPoint=p;dragging=true;return;}
 if(lv_event_get_code(e)==LV_EVENT_RELEASED||lv_event_get_code(e)==LV_EVENT_PRESS_LOST){dragging=false;return;}
 if(!dragging)return;
 int dx=p.x-dragPoint.x,dy=p.y-dragPoint.y;
 if(dx||dy){followBoat=false;mapProjection_pan(centerLat,centerLon,zoomLevel,-dx,-dy);dragPoint=p;}
}
void mapRenderer_create(lv_obj_t *viewport){
 if(!viewport||view)return;
 view=viewport;lv_obj_add_event_cb(view,viewportDeleted,LV_EVENT_DELETE,nullptr);zoomLevel=lakeStorage_getZoom();viewW=lv_obj_get_width(view);viewH=lv_obj_get_height(view);
 lv_obj_set_style_bg_color(view,missingColor(),LV_PART_MAIN);lv_obj_set_style_pad_all(view,0,LV_PART_MAIN);lv_obj_set_style_clip_corner(view,true,LV_PART_MAIN);lv_obj_clear_flag(view,LV_OBJ_FLAG_SCROLLABLE);lv_obj_add_flag(view,LV_OBJ_FLAG_CLICKABLE);
 lv_obj_add_event_cb(view,panEvent,LV_EVENT_PRESSED,nullptr);lv_obj_add_event_cb(view,panEvent,LV_EVENT_PRESSING,nullptr);lv_obj_add_event_cb(view,panEvent,LV_EVENT_RELEASED,nullptr);lv_obj_add_event_cb(view,panEvent,LV_EVENT_PRESS_LOST,nullptr);
 for(uint8_t i=0;i<TILE_SLOTS;i++){tiles[i].image=lv_image_create(view);lv_obj_set_size(tiles[i].image,256,256);lv_obj_add_flag(tiles[i].image,LV_OBJ_FLAG_HIDDEN);tiles[i].valid=false;}
 route=lv_line_create(view);lv_line_set_points(route,routePoints,2);lv_obj_set_style_line_color(route,lv_color_hex(0xFFB300),LV_PART_MAIN);lv_obj_set_style_line_width(route,3,LV_PART_MAIN);lv_obj_set_style_line_opa(route,LV_OPA_COVER,LV_PART_MAIN);lv_obj_add_flag(route,LV_OBJ_FLAG_HIDDEN);
 boat=lv_label_create(view);lv_label_set_text(boat,"▲");lv_obj_set_style_text_color(boat,lv_color_hex(0x00D5FF),LV_PART_MAIN);lv_obj_set_style_text_font(boat,LV_FONT_DEFAULT,LV_PART_MAIN);lv_obj_add_flag(boat,LV_OBJ_FLAG_HIDDEN);
 home=lv_label_create(view);lv_label_set_text(home,"H");lv_obj_set_style_text_color(home,lv_color_hex(0xFFD740),LV_PART_MAIN);lv_obj_set_style_bg_color(home,lv_color_hex(0x162A38),LV_PART_MAIN);lv_obj_set_style_bg_opa(home,LV_OPA_COVER,LV_PART_MAIN);lv_obj_add_flag(home,LV_OBJ_FLAG_HIDDEN);
 for(uint8_t i=0;i<16;i++){wps[i]=lv_label_create(view);lv_label_set_text(wps[i],"WP");lv_obj_set_style_text_color(wps[i],lv_color_hex(0x7CFC00),LV_PART_MAIN);lv_obj_set_style_bg_color(wps[i],lv_color_hex(0x162A38),LV_PART_MAIN);lv_obj_set_style_bg_opa(wps[i],LV_OPA_COVER,LV_PART_MAIN);lv_obj_set_style_pad_all(wps[i],2,LV_PART_MAIN);lv_obj_add_flag(wps[i],LV_OBJ_FLAG_HIDDEN);}
 attributionLabel=lv_label_create(view);lv_obj_set_style_text_color(attributionLabel,lv_color_hex(0xFFFFFF),LV_PART_MAIN);lv_obj_set_style_bg_color(attributionLabel,lv_color_hex(0x101820),LV_PART_MAIN);lv_obj_set_style_bg_opa(attributionLabel,LV_OPA_80,LV_PART_MAIN);lv_obj_set_style_pad_all(attributionLabel,2,LV_PART_MAIN);lv_label_set_long_mode(attributionLabel,LV_LABEL_LONG_DOT);lv_obj_add_flag(attributionLabel,LV_OBJ_FLAG_HIDDEN);
}
void mapRenderer_update(bool gpsValid,float boatLat,float boatLon,float heading,bool homeValid,float homeLat,float homeLon,const MapRendererWaypoint *waypoints,uint8_t selectedIndex,bool returnHome){
 if(!view)return;
 (void)heading;if(offlineMap_hasMapData()&&mapStyle!=offlineMap_getStyle()){mapStyle=offlineMap_getStyle();for(auto&t:tiles)releaseTile(t);}
 if(followBoat)haveCenter=false;viewW=lv_obj_get_width(view);viewH=lv_obj_get_height(view);bool validCenter=gpsValid;if(!followBoat&&haveCenter){}else if(validCenter){centerLat=boatLat;centerLon=boatLon;haveCenter=true;}else if(homeValid){centerLat=homeLat;centerLon=homeLon;haveCenter=true;}else{for(uint8_t i=0;i<16;i++)if(waypoints&&waypoints[i].valid){centerLat=waypoints[i].lat;centerLon=waypoints[i].lon;haveCenter=true;break;}}
 drawTiles();
 if(attributionLabel){
  char attribution[128]={0};
  if(offlineMap_getAttribution(attribution,sizeof(attribution))){
   lv_label_set_text(attributionLabel,attribution);lv_obj_set_size(attributionLabel,viewW,20);lv_obj_set_pos(attributionLabel,0,viewH-20);lv_obj_remove_flag(attributionLabel,LV_OBJ_FLAG_HIDDEN);
  }else lv_obj_add_flag(attributionLabel,LV_OBJ_FLAG_HIDDEN);
 }
 if(gpsValid){int x,y;if(projectPoint(boatLat,boatLon,x,y)){lv_obj_set_pos(boat,x-10,y-12);lv_obj_remove_flag(boat,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(boat,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(boat,LV_OBJ_FLAG_HIDDEN);
 if(homeValid){int x,y;if(projectPoint(homeLat,homeLon,x,y)){lv_obj_set_pos(home,x-6,y-8);lv_obj_remove_flag(home,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(home,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(home,LV_OBJ_FLAG_HIDDEN);
 for(uint8_t i=0;i<16;i++){if(waypoints&&waypoints[i].valid){char text[12];snprintf(text,sizeof(text),"W%02u%s%s",i+1,waypoints[i].bait?"B":"",waypoints[i].hook?"H":"");lv_label_set_text(wps[i],text);int x,y;if(projectPoint(waypoints[i].lat,waypoints[i].lon,x,y)){lv_obj_set_pos(wps[i],x-8,y-8);lv_obj_remove_flag(wps[i],LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(wps[i],LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(wps[i],LV_OBJ_FLAG_HIDDEN);}
 int boatX=0,boatY=0;bool boatVisible=gpsValid&&projectPoint(boatLat,boatLon,boatX,boatY);routePoints[0].x=boatX;routePoints[0].y=boatY;bool target=false;float targetLat=0,targetLon=0;
 if(returnHome&&homeValid){target=true;targetLat=homeLat;targetLon=homeLon;}else if(selectedIndex<16&&waypoints&&waypoints[selectedIndex].valid){target=true;targetLat=waypoints[selectedIndex].lat;targetLon=waypoints[selectedIndex].lon;}
 if(boatVisible&&target){int x,y;if(projectPoint(targetLat,targetLon,x,y)){routePoints[1].x=x;routePoints[1].y=y;lv_line_set_points(route,routePoints,2);lv_obj_remove_flag(route,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(route,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(route,LV_OBJ_FLAG_HIDDEN);
}
void mapRenderer_zoomIn(){if(zoomLevel<MAX_ZOOM){zoomLevel++;lakeStorage_setZoom(zoomLevel);for(auto&t:tiles)releaseTile(t);drawTiles();}}
void mapRenderer_zoomOut(){if(zoomLevel>MIN_ZOOM){zoomLevel--;lakeStorage_setZoom(zoomLevel);for(auto&t:tiles)releaseTile(t);drawTiles();}}
uint8_t mapRenderer_getZoom(){return zoomLevel;}
void mapRenderer_setStyle(OfflineMapStyle style){if(style!=mapStyle){mapStyle=style;for(auto&t:tiles)releaseTile(t);drawTiles();}}

void mapRenderer_invalidateTiles(){for(auto&t:tiles)releaseTile(t);drawTiles();}

void mapRenderer_resetProfile(){haveCenter=false;followBoat=true;dragging=false;zoomLevel=lakeStorage_getZoom();for(auto&t:tiles){releaseTile(t);hideTile(t);}if(route)lv_obj_add_flag(route,LV_OBJ_FLAG_HIDDEN);if(home)lv_obj_add_flag(home,LV_OBJ_FLAG_HIDDEN);for(auto *w:wps)if(w)lv_obj_add_flag(w,LV_OBJ_FLAG_HIDDEN);}

void mapRenderer_recenter(){followBoat=true;dragging=false;}
