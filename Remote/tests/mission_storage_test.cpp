#include "mission_storage.h"
#include "lake_storage.h"
#include "boat_state.h"
#include <SD.h>
#include <cassert>
#include <cmath>
#include <iostream>
MockSD SD;
BoatState boatState={};
static StoredMission record(uint8_t lake){
 StoredMission r={};r.magic=STORE_MAGIC;r.homeValid=true;r.homeLat=42+lake*0.1f;r.homeLon=25;r.selected=15;
 for(uint8_t i=0;i<SLOT_COUNT;++i)r.points[i]={true,42+lake*0.1f,25+i*0.01f,(i%2)==0,(i%2)!=0};
 r.crc=missionStorage_crc32(reinterpret_cast<const uint8_t*>(&r),offsetof(StoredMission,crc));return r;
}
int main(){
 boatState.mode=MODE_MANUAL;boatState.radioConnected=boatState.emergencyStop=boatState.emergencyStopConfirmed=true;
 assert(lakeStorage_begin(true));StoredMission got={};
 for(uint8_t i=0;i<8;++i){assert(lakeStorage_select(i));assert(!missionStorage_read(got));assert(missionStorage_write(record(i)));}
 assert(lakeStorage_begin(true));
 for(uint8_t i=0;i<8;++i){assert(lakeStorage_select(i));assert(missionStorage_read(got));auto expected=record(i);assert(got.homeLat==expected.homeLat&&got.selected==15);for(uint8_t j=0;j<SLOT_COUNT;++j){assert(got.points[j].valid&&got.points[j].lat==expected.points[j].lat&&got.points[j].lon==expected.points[j].lon&&got.points[j].baitMark==expected.points[j].baitMark&&got.points[j].hookMark==expected.points[j].hookMark);}}
 assert(lakeStorage_select(0));auto next=record(1);
 SD.failRenameFrom="/Lake_01/waypoints.tmp";assert(!missionStorage_write(next));assert(missionStorage_read(got)&&got.homeLat==record(0).homeLat);SD.failRenameFrom.clear();
 assert(missionStorage_write(next));SD.files["/Lake_01/waypoints.bin"][0]^=1;assert(missionStorage_read(got)&&got.homeLat==record(0).homeLat);
 SD.shortWrite=true;assert(!missionStorage_write(next));assert(missionStorage_read(got));SD.shortWrite=false;
 next.points[15].lat=NAN;next.homeLat=91;missionStorage_sanitize(next);assert(!next.points[15].valid&&!next.homeValid&&next.selected==-1);
 SD.mounted=false;assert(!missionStorage_read(got));assert(!missionStorage_write(next));
 std::cout<<"128 waypoint slots, CRC, corrupt/short writes and per-lake Home tested\n";
}
