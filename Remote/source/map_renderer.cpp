#include "map_renderer.h"
#include "offline_map.h"
#include "map_projection.h"
#include <Arduino.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

static constexpr uint8_t TILE_SLOTS=9;
static constexpr uint8_t MIN_ZOOM=5,MAX_ZOOM=19;
struct TileSlot { int32_t x,y; uint8_t z; OfflineMapStyle style; uint16_t *pixels; lv_image_dsc_t dsc; lv_obj_t *image; bool valid; };
static TileSlot tiles[TILE_SLOTS]={};
static lv_obj_t *view=nullptr,*boat=nullptr,*home=nullptr,*wps[16]={},*route=nullptr,*zoomText=nullptr;
static lv_point_t routePoints[2];
static uint8_t zoomLevel=15; static OfflineMapStyle mapStyle=OFFLINE_MAP_STREET;
static double centerLat=0,centerLon=0; static bool haveCenter=false;
static int viewW=0,viewH=0;
static lv_color_t missingColor(){return lv_color_hex(0x263746);}
static void hideTile(TileSlot &t){if(t.image)lv_obj_add_flag(t.image,LV_OBJ_FLAG_HIDDEN);}
static void releaseTile(TileSlot &t){if(t.image&&t.pixels)lv_image_set_src(t.image,nullptr);if(t.pixels)offlineMap_freeTile(t.pixels);t.pixels=nullptr;t.valid=false;}
static TileSlot *findTile(int32_t x,int32_t y,uint8_t z){for(auto &t:tiles)if(t.valid&&t.x==x&&t.y==y&&t.z==z&&t.style==mapStyle)return &t;return nullptr;}
static TileSlot *allocateTile(int32_t x,int32_t y,uint8_t z){
 TileSlot *t=findTile(x,y,z);if(t)return t;
 for(auto &candidate:tiles)if(!candidate.valid){t=&candidate;break;}
 if(!t)t=&tiles[0];releaseTile(*t);
 uint16_t *buf=offlineMap_readTile(mapStyle,z,(uint32_t)x,(uint32_t)y);
 t->pixels=buf;t->x=x;t->y=y;t->z=z;t->style=mapStyle;t->valid=true;if(!buf){lv_obj_add_flag(t->image,LV_OBJ_FLAG_HIDDEN);return t;}
 memset(&t->dsc,0,sizeof(t->dsc));t->dsc.header.cf=LV_COLOR_FORMAT_RGB565;t->dsc.header.w=OFFLINE_MAP_TILE_SIZE;t->dsc.header.h=OFFLINE_MAP_TILE_SIZE;t->dsc.data_size=OFFLINE_MAP_TILE_BYTES;t->dsc.data=(const uint8_t*)buf;
 lv_image_set_src(t->image,&t->dsc);lv_obj_remove_flag(t->image,LV_OBJ_FLAG_HIDDEN);return t;
}
static void drawTiles(){
 if(!view||!haveCenter||!offlineMap_hasMapData()){for(auto&t:tiles)hideTile(t);return;}
 MapWorldPixel c=mapProjection_toWorldPixel(centerLat,centerLon,zoomLevel);
 double left=c.x-viewW/2.0,top=c.y-viewH/2.0;
 int32_t firstX=(int32_t)floor(left/256.0),firstY=(int32_t)floor(top/256.0);
 for(auto&t:tiles)hideTile(t);
 uint8_t slot=0;const int32_t n=(1L<<zoomLevel);
 for(int dy=0;dy<3;dy++)for(int dx=0;dx<3;dx++){
  int32_t displayX=firstX+dx,ty=firstY+dy;if(ty<0||ty>=n)continue;int32_t tileX=(displayX%n+n)%n;
  TileSlot*t=allocateTile(tileX,ty,zoomLevel);if(!t||!t->pixels)continue;
  lv_obj_set_pos(t->image,(int)round(displayX*256.0-left),(int)round(ty*256.0-top));lv_obj_remove_flag(t->image,LV_OBJ_FLAG_HIDDEN);slot++;
 }
 (void)slot;
}
static bool projectPoint(float lat,float lon,int &x,int &y){
 if(!haveCenter)return false;MapWorldPixel c=mapProjection_toWorldPixel(centerLat,centerLon,zoomLevel),p=mapProjection_toWorldPixel(lat,lon,zoomLevel);
 double dx=p.x-c.x,dy=p.y-c.y;if(dx>=(double)(1UL<<zoomLevel)*256/2)dx-=((double)(1UL<<zoomLevel)*256);if(dx< -((double)(1UL<<zoomLevel)*256/2))dx+=((double)(1UL<<zoomLevel)*256);
 x=(int)round(viewW/2.0+dx);y=(int)round(viewH/2.0+dy);return x>=-24&&x<=viewW+24&&y>=-24&&y<=viewH+24;
}
void mapRenderer_create(lv_obj_t *viewport){
 view=viewport;viewW=lv_obj_get_width(view);viewH=lv_obj_get_height(view);
 lv_obj_set_style_bg_color(view,missingColor(),LV_PART_MAIN);lv_obj_set_style_pad_all(view,0,LV_PART_MAIN);lv_obj_set_style_clip_corner(view,true,LV_PART_MAIN);lv_obj_clear_flag(view,LV_OBJ_FLAG_SCROLLABLE);
 for(uint8_t i=0;i<TILE_SLOTS;i++){tiles[i].image=lv_image_create(view);lv_obj_set_size(tiles[i].image,256,256);lv_obj_add_flag(tiles[i].image,LV_OBJ_FLAG_HIDDEN);tiles[i].valid=false;}
 route=lv_line_create(view);lv_line_set_points(route,routePoints,2);lv_obj_set_style_line_color(route,lv_color_hex(0xFFB300),LV_PART_MAIN);lv_obj_set_style_line_width(route,3,LV_PART_MAIN);lv_obj_set_style_line_opa(route,LV_OPA_COVER,LV_PART_MAIN);lv_obj_add_flag(route,LV_OBJ_FLAG_HIDDEN);
 boat=lv_label_create(view);lv_label_set_text(boat,"▲");lv_obj_set_style_text_color(boat,lv_color_hex(0x00D5FF),LV_PART_MAIN);lv_obj_set_style_text_font(boat,LV_FONT_DEFAULT,LV_PART_MAIN);lv_obj_add_flag(boat,LV_OBJ_FLAG_HIDDEN);
 home=lv_label_create(view);lv_label_set_text(home,"H");lv_obj_set_style_text_color(home,lv_color_hex(0xFFD740),LV_PART_MAIN);lv_obj_set_style_bg_color(home,lv_color_hex(0x162A38),LV_PART_MAIN);lv_obj_set_style_bg_opa(home,LV_OPA_COVER,LV_PART_MAIN);lv_obj_add_flag(home,LV_OBJ_FLAG_HIDDEN);
 for(uint8_t i=0;i<16;i++){wps[i]=lv_label_create(view);lv_label_set_text(wps[i],"WP");lv_obj_set_style_text_color(wps[i],lv_color_hex(0x7CFC00),LV_PART_MAIN);lv_obj_set_style_bg_color(wps[i],lv_color_hex(0x162A38),LV_PART_MAIN);lv_obj_set_style_bg_opa(wps[i],LV_OPA_COVER,LV_PART_MAIN);lv_obj_set_style_pad_all(wps[i],2,LV_PART_MAIN);lv_obj_add_flag(wps[i],LV_OBJ_FLAG_HIDDEN);}
}
void mapRenderer_update(bool gpsValid,float boatLat,float boatLon,float heading,bool homeValid,float homeLat,float homeLon,const MapRendererWaypoint *waypoints,uint8_t selectedIndex,bool returnHome){
 (void)heading;viewW=lv_obj_get_width(view);viewH=lv_obj_get_height(view);bool validCenter=gpsValid;if(validCenter){centerLat=boatLat;centerLon=boatLon;haveCenter=true;}else if(homeValid){centerLat=homeLat;centerLon=homeLon;haveCenter=true;}else{for(uint8_t i=0;i<16;i++)if(waypoints&&waypoints[i].valid){centerLat=waypoints[i].lat;centerLon=waypoints[i].lon;haveCenter=true;break;}}
 drawTiles();
 if(gpsValid){int x,y;if(projectPoint(boatLat,boatLon,x,y)){lv_obj_set_pos(boat,x-10,y-12);lv_obj_remove_flag(boat,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(boat,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(boat,LV_OBJ_FLAG_HIDDEN);
 if(homeValid){int x,y;if(projectPoint(homeLat,homeLon,x,y)){lv_obj_set_pos(home,x-6,y-8);lv_obj_remove_flag(home,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(home,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(home,LV_OBJ_FLAG_HIDDEN);
 for(uint8_t i=0;i<16;i++){if(waypoints&&waypoints[i].valid){char text[12];snprintf(text,sizeof(text),"W%02u%s%s",i+1,waypoints[i].bait?"B":"",waypoints[i].hook?"H":"");lv_label_set_text(wps[i],text);int x,y;if(projectPoint(waypoints[i].lat,waypoints[i].lon,x,y)){lv_obj_set_pos(wps[i],x-8,y-8);lv_obj_remove_flag(wps[i],LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(wps[i],LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(wps[i],LV_OBJ_FLAG_HIDDEN);}
 routePoints[0].x=viewW/2;routePoints[0].y=viewH/2;bool target=false;float targetLat=0,targetLon=0;
 if(returnHome&&homeValid){target=true;targetLat=homeLat;targetLon=homeLon;}else if(selectedIndex<16&&waypoints&&waypoints[selectedIndex].valid){target=true;targetLat=waypoints[selectedIndex].lat;targetLon=waypoints[selectedIndex].lon;}
 if(gpsValid&&target){int x,y;if(projectPoint(targetLat,targetLon,x,y)){routePoints[1].x=x;routePoints[1].y=y;lv_line_set_points(route,routePoints,2);lv_obj_remove_flag(route,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(route,LV_OBJ_FLAG_HIDDEN);}else lv_obj_add_flag(route,LV_OBJ_FLAG_HIDDEN);
}
void mapRenderer_zoomIn(){if(zoomLevel<MAX_ZOOM){zoomLevel++;for(auto&t:tiles)releaseTile(t);drawTiles();}}
void mapRenderer_zoomOut(){if(zoomLevel>MIN_ZOOM){zoomLevel--;for(auto&t:tiles)releaseTile(t);drawTiles();}}
uint8_t mapRenderer_getZoom(){return zoomLevel;}
void mapRenderer_setStyle(OfflineMapStyle style){if(style!=mapStyle){mapStyle=style;for(auto&t:tiles)releaseTile(t);drawTiles();}}
