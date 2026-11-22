#pragma once
#include <stdint.h>
#include <string.h>

// Fixed 24-byte little-endian NVS record. No compiler padding is persisted.
static const uint32_t BOAT_HOME_POINT_MAGIC = 0x454D4F48U; // bytes "HOME"
static const uint16_t BOAT_HOME_POINT_VERSION = 1U;
static const uint16_t BOAT_HOME_POINT_RECORD_BYTES = 24U;

struct BoatHomePoint {
 int32_t latE7;
 int32_t lonE7;
 uint32_t sequence;
};

struct BoatHomePointStore {
 bool (*load)(BoatHomePoint &out);
 bool (*save)(const BoatHomePoint &point);
};

inline bool boatHomeCoordinatesValid(int32_t latE7,int32_t lonE7){
 return latE7>=-900000000&&latE7<=900000000&&
        lonE7>=-1800000000&&lonE7<=1800000000;
}
inline bool boatHomePointValid(const BoatHomePoint &p){
 return p.sequence!=0&&boatHomeCoordinatesValid(p.latE7,p.lonE7);
}
inline bool boatHomePointEqual(const BoatHomePoint &a,const BoatHomePoint &b){
 return a.latE7==b.latE7&&a.lonE7==b.lonE7&&a.sequence==b.sequence;
}
inline bool boatHomeSequenceNewer(uint32_t a,uint32_t b){
 return (int32_t)(a-b)>0;
}
inline void boatHomePutLe16(uint8_t *p,uint16_t v){
 p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);
}
inline void boatHomePutLe32(uint8_t *p,uint32_t v){
 for(uint8_t i=0;i<4;++i)p[i]=(uint8_t)(v>>(8U*i));
}
inline uint16_t boatHomeGetLe16(const uint8_t *p){
 return (uint16_t)p[0]|((uint16_t)p[1]<<8);
}
inline uint32_t boatHomeGetLe32(const uint8_t *p){
 return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
inline uint32_t boatHomeCrc32(const uint8_t *p,uint32_t n){
 uint32_t c=0xFFFFFFFFU;
 for(uint32_t i=0;i<n;++i){
  c^=p[i];
  for(uint8_t j=0;j<8;++j)c=(c>>1)^((c&1U)?0xEDB88320U:0U);
 }
 return ~c;
}
inline bool boatHomeRecordEncode(const BoatHomePoint &point,uint8_t out[BOAT_HOME_POINT_RECORD_BYTES]){
 if(!out||!boatHomePointValid(point))return false;
 memset(out,0,BOAT_HOME_POINT_RECORD_BYTES);
 boatHomePutLe32(out,BOAT_HOME_POINT_MAGIC);
 boatHomePutLe16(out+4,BOAT_HOME_POINT_VERSION);
 boatHomePutLe16(out+6,BOAT_HOME_POINT_RECORD_BYTES);
 boatHomePutLe32(out+8,(uint32_t)point.latE7);
 boatHomePutLe32(out+12,(uint32_t)point.lonE7);
 boatHomePutLe32(out+16,point.sequence);
 boatHomePutLe32(out+20,boatHomeCrc32(out,20));
 return true;
}
inline bool boatHomeRecordDecode(const uint8_t in[BOAT_HOME_POINT_RECORD_BYTES],BoatHomePoint &out){
 if(!in||boatHomeGetLe32(in)!=BOAT_HOME_POINT_MAGIC||
    boatHomeGetLe16(in+4)!=BOAT_HOME_POINT_VERSION||
    boatHomeGetLe16(in+6)!=BOAT_HOME_POINT_RECORD_BYTES||
    boatHomeGetLe32(in+20)!=boatHomeCrc32(in,20))return false;
 BoatHomePoint p={(int32_t)boatHomeGetLe32(in+8),(int32_t)boatHomeGetLe32(in+12),boatHomeGetLe32(in+16)};
 if(!boatHomePointValid(p))return false;
 out=p;return true;
}
