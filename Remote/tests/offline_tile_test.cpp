#include "offline_map.h"
#include "lake_storage.h"
#include "boat_state.h"
#include <SD.h>
#include <cassert>
#include <iostream>
MockSD SD;
BoatState boatState={};
static void put(const char *path,const std::string &v){SD.files[path]={v.begin(),v.end()};}
int main(){
 const char *tile="/Lake_01/maps/street/0/0/0.rgb565";
 put("/Lake_01/maps/manifest.txt","FLYTEC_MAP1\nstyle=street\npack_complete=true\nattribution=Test\nlicence_url=https://example.test\nzoom_levels=0\nexported_tile_count=1\n");
 put("/Lake_01/maps/coverage_index.csv","z,x,y,present\n0,0,0,1\n");
 SD.files[tile]=std::vector<uint8_t>(OFFLINE_MAP_TILE_BYTES,0x5a);
 assert(lakeStorage_begin(true)&&offlineMap_begin(true));
 assert(!offlineMap_startTile(OFFLINE_MAP_SATELLITE,0,0,0));
 assert(!offlineMap_startTile(OFFLINE_MAP_STREET,0,1,0));
 uint16_t *p=nullptr;
 assert(offlineMap_startTile(OFFLINE_MAP_STREET,0,0,0));
 for(int i=0;i<31;i++){assert(offlineMap_stepTile(p)==0&&!p);}
 assert(offlineMap_stepTile(p)==1&&p&&p[0]==0x5a5a&&p[65535]==0x5a5a);
 offlineMap_freeTile(p);p=nullptr;
 assert(offlineMap_stepTile(p)==-1&&!p);
 assert(offlineMap_startTile(OFFLINE_MAP_STREET,0,0,0));
 assert(offlineMap_stepTile(p)==0);offlineMap_cancelTile();
 assert(offlineMap_stepTile(p)==-1&&!p);
 assert(offlineMap_startTile(OFFLINE_MAP_STREET,0,0,0));
 SD.files[tile].resize(100);
 assert(offlineMap_stepTile(p)==-1&&!p);
 assert(!offlineMap_startTile(OFFLINE_MAP_STREET,0,0,0));
 SD.files[tile].resize(OFFLINE_MAP_TILE_BYTES,0x5a);
 assert(offlineMap_startTile(OFFLINE_MAP_STREET,0,0,0));SD.mounted=false;
 assert(offlineMap_stepTile(p)==-1&&!p);
 SD.mounted=true;assert(offlineMap_begin(true));
 assert(offlineMap_startTile(OFFLINE_MAP_STREET,0,0,0));
 assert(offlineMap_begin(true));assert(offlineMap_stepTile(p)==-1&&!p);
 assert(offlineMap_startTile(OFFLINE_MAP_STREET,0,0,0));offlineMap_cancelTile();
 std::cout<<"Cooperative offline tile loader fault cases passed\n";
}
