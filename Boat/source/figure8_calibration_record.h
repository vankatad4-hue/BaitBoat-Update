#pragma once
#include <stdint.h>
#include <cmath>
#include <string.h>
// Portable, versioned calibration record. Storage backend MUST provide
// atomic dual-slot write/read and only activate after successful validation.
struct Figure8StoredCalibration{
 uint32_t magic=0x46384341U; // F8CA
 uint16_t version=1;
 uint16_t bytes=sizeof(Figure8StoredCalibration);
 float gyOffsetX=0,gyOffsetY=0,gyScaleX=1,gyScaleY=1;
 float m100OffsetX=0,m100OffsetY=0,m100ScaleX=1,m100ScaleY=1;
 uint32_t sequence=0,crc=0;
};
inline uint32_t figure8Crc32(const uint8_t *p,uint32_t n){
 uint32_t c=0xFFFFFFFFU;
 for(uint32_t i=0;i<n;++i){
  c^=p[i];
  for(unsigned j=0;j<8;++j)c=(c>>1)^((c&1U)?0xEDB88320U:0U);
 }
 return ~c;
}
// Version 1 on-disk record is exactly 48 bytes, little-endian IEEE-754.
// Build the CRC input field by field rather than hashing compiler padding,
// object representation or a transient nonzero CRC member.
static_assert(sizeof(float)==4,"Figure-eight record needs binary32 floats");
static_assert(sizeof(Figure8StoredCalibration)==48,"Figure-eight record layout changed; bump the format version");
inline void figure8PutLe16(uint8_t *out,uint16_t value){
 out[0]=(uint8_t)value;out[1]=(uint8_t)(value>>8);
}
inline void figure8PutLe32(uint8_t *out,uint32_t value){
 for(unsigned i=0;i<4;++i)out[i]=(uint8_t)(value>>(8U*i));
}
inline uint32_t figure8RecordCrc(const Figure8StoredCalibration &r){
 uint8_t b[48]={};
 figure8PutLe32(b,r.magic);
 figure8PutLe16(b+4,r.version);
 figure8PutLe16(b+6,r.bytes);
 const float v[8]={r.gyOffsetX,r.gyOffsetY,r.gyScaleX,r.gyScaleY,
                   r.m100OffsetX,r.m100OffsetY,r.m100ScaleX,r.m100ScaleY};
 for(unsigned i=0;i<8;++i){
  uint32_t bits=0;memcpy(&bits,&v[i],sizeof(bits));
  figure8PutLe32(b+8+4*i,bits);
 }
 figure8PutLe32(b+40,r.sequence);
 // b[44..47] stays zero so CRC never includes its stored checksum.
 return figure8Crc32(b,sizeof(b));
}
inline bool figure8CalibrationValid(const Figure8StoredCalibration &r){
 if(r.magic!=0x46384341U||r.version!=1||r.bytes!=sizeof(r)||
    r.sequence==0||r.crc!=figure8RecordCrc(r))return false;
 const float values[]={r.gyOffsetX,r.gyOffsetY,r.gyScaleX,r.gyScaleY,
                       r.m100OffsetX,r.m100OffsetY,r.m100ScaleX,r.m100ScaleY};
 for(float v:values)if(!std::isfinite(v))return false;
 return r.gyScaleX>0.1f&&r.gyScaleX<10.f&&r.gyScaleY>0.1f&&r.gyScaleY<10.f&&
        r.m100ScaleX>0.1f&&r.m100ScaleX<10.f&&r.m100ScaleY>0.1f&&r.m100ScaleY<10.f;
}
inline bool figure8SelectRecord(const Figure8StoredCalibration &a,
                                const Figure8StoredCalibration &b,
                                Figure8StoredCalibration &out){
 const bool av=figure8CalibrationValid(a),bv=figure8CalibrationValid(b);
 if(!av&&!bv)return false;
 out=av&&(!bv||(int32_t)(a.sequence-b.sequence)>0)?a:b;return true;
}
