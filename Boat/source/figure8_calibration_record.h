#pragma once
#include <stdint.h>
#include <cmath>
#include "figure8_compass_calibration.h"
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
inline uint32_t figure8RecordCrc(Figure8StoredCalibration r){
 r.crc=0;return figure8Crc32(reinterpret_cast<const uint8_t*>(&r),sizeof(r));
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
