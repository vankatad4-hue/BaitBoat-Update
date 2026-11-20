#include "mission_storage.h"
#include "lake_storage.h"
#include <SD.h>
#include <cmath>
#include <cstring>
uint32_t missionStorage_crc32(const uint8_t *data,size_t len){uint32_t crc=0xFFFFFFFFUL;for(size_t i=0;i<len;i++){crc^=data[i];for(uint8_t b=0;b<8;b++)crc=(crc&1)?(crc>>1)^0xEDB88320UL:crc>>1;}return ~crc;}
bool missionStorage_validCoordinate(float lat,float lon){return std::isfinite(lat)&&std::isfinite(lon)&&lat>=-90&&lat<=90&&lon>=-180&&lon<=180;}
bool missionStorage_readRecord(const char *path, StoredMission &record){
 File in=SD.open(path,FILE_READ);if(!in)return false;
 uint8_t raw[sizeof(StoredMission)]={};
 bool ok=in.size()==sizeof(raw)&&in.read(raw,sizeof(raw))==sizeof(raw);in.close();
 if(!ok)return false;
 for(size_t i=0;i<SLOT_COUNT;++i){size_t offset=offsetof(StoredMission,points)+i*sizeof(WaypointSlot);if(raw[offset+offsetof(WaypointSlot,valid)]>1||raw[offset+offsetof(WaypointSlot,baitMark)]>1||raw[offset+offsetof(WaypointSlot,hookMark)]>1)return false;}
 if(raw[offsetof(StoredMission,homeValid)]>1)return false;
 memcpy(&record,raw,sizeof(record));
 return ok&&record.magic==STORE_MAGIC&&record.crc==missionStorage_crc32(reinterpret_cast<const uint8_t*>(&record),offsetof(StoredMission,crc));
}
bool missionStorage_write(const StoredMission &record){
 if(!lakeStorage_ready())return false;
 String tmp=lakeStorage_path("/waypoints.tmp"), dst=lakeStorage_path("/waypoints.bin"), bak=lakeStorage_path("/waypoints.bak");
 if(SD.exists(tmp.c_str())&&!SD.remove(tmp.c_str()))return false;
 File out=SD.open(tmp.c_str(),FILE_WRITE);if(!out)return false;
 size_t written=out.write(reinterpret_cast<const uint8_t*>(&record),sizeof(record));out.flush();out.close();
 StoredMission check={};
 if(written!=sizeof(record)||!missionStorage_readRecord(tmp.c_str(),check))return false;
 if(SD.exists(dst.c_str())){
  if(missionStorage_readRecord(dst.c_str(),check)){
   if(SD.exists(bak.c_str())&&!SD.remove(bak.c_str()))return false;
   if(!SD.rename(dst.c_str(),bak.c_str()))return false;
  }else if(!SD.remove(dst.c_str()))return false;
 }
 return SD.rename(tmp.c_str(),dst.c_str());
}
bool missionStorage_read(StoredMission &record){
 if(!sdMissionAvailable())return false;
 return missionStorage_readRecord(lakeStorage_path("/waypoints.bin").c_str(),record)||
        missionStorage_readRecord(lakeStorage_path("/waypoints.bak").c_str(),record);
}

void missionStorage_sanitize(StoredMission &record){
 for(uint8_t i=0;i<SLOT_COUNT;++i)
  if(!record.points[i].valid||!missionStorage_validCoordinate(record.points[i].lat,record.points[i].lon))record.points[i]={};
 if(!record.homeValid||!missionStorage_validCoordinate(record.homeLat,record.homeLon)){record.homeValid=false;record.homeLat=record.homeLon=0;}
 if(record.selected<0||record.selected>=SLOT_COUNT||!record.points[(uint8_t)record.selected].valid)record.selected=-1;
 record.crc=missionStorage_crc32(reinterpret_cast<const uint8_t*>(&record),offsetof(StoredMission,crc));
}
