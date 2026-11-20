#include "map_copy.h"
#include "lake_storage.h"
#include "boat_state.h"
#include <cassert>
#include <iostream>
MockSD SD;
BoatState boatState={};
int main(){
 assert(lakeStorage_begin(true));
 const char *src="/source",*dst="/Lake_01/maps-alt/tile";
 SD.files[src]=std::vector<uint8_t>(8193,42);
 uint32_t copied=0;
 assert(!mapCopy_start(src,dst,8192,0));
 assert(!mapCopy_start(src,dst,9000,8192));
 assert(mapCopy_start(src,dst,9000,8193));
 assert(mapCopy_step(copied,8193)==0&&copied==4096);
 assert(mapCopy_step(copied,8193)==0&&copied==8192);
 assert(mapCopy_step(copied,8193)==1&&copied==8193);
 assert(!mapCopy_active()&&SD.files[src]==SD.files[dst]);
 copied=0;SD.shortWrite=true;
 assert(mapCopy_start(src,dst,9000,8193));
 assert(mapCopy_step(copied,8193)==-1&&!mapCopy_active()&&copied==0);
 SD.shortWrite=false;
 assert(mapCopy_start(src,dst,9000,8193));
 assert(mapCopy_step(copied,4095)==-1&&!mapCopy_active());
 assert(mapCopy_start(src,dst,9000,8193));
 assert(mapCopy_step(copied,8193)==0);mapCopy_cancel();
 assert(!mapCopy_active());assert(lakeStorage_mapRoot()==lakeStorage_path("/maps"));
 copied=0;assert(mapCopy_start(src,dst,9000,8193));
 SD.files[src].resize(100);
 assert(mapCopy_step(copied,8193)==-1&&!mapCopy_active());
 std::cout<<"Map copy fault cases passed\n";
}
