#pragma once
#include <stdint.h>
#include <cmath>
// HGLRC M100 PRO: UBX NAV-PVT (class 01, ID 07), 92-byte payload.
// Bounded parser with checksum; does not accept NMEA as UBX.
struct M100UbxFix {
 int32_t latE7=0,lonE7=0;
 double accuracyM=0;
 uint16_t speedCmS=0;
 uint8_t fixType=0,satellites=0;
 uint32_t at=0;
 bool positionValid=false,speedValid=false;
};
class M100UbxNavPvt {
public:
 void feed(uint8_t b,uint32_t now){
  switch(state_){
   case 0:if(b==0xb5)state_=1;break;
   case 1:state_=b==0x62?2:(b==0xb5?1:0);break;
   case 2:cls_=b;ckA_=ckB_=0;check(b);state_=3;break;
   case 3:id_=b;check(b);state_=4;break;
   case 4:len_=b;check(b);state_=5;break;
   case 5:len_|=(uint16_t)b<<8;check(b);idx_=0;state_=(len_==92)?6:0;break;
   case 6:payload_[idx_++]=b;check(b);if(idx_==92)state_=7;break;
   case 7:if(b!=ckA_){state_=0;break;}state_=8;break;
   case 8:if(b==ckB_&&cls_==1&&id_==7)decode(now);state_=0;break;
  }
 }
 M100UbxFix snapshot(uint32_t now)const{
  M100UbxFix out=fix_;
  if((uint32_t)(now-out.at)>=2000U){out.positionValid=false;out.speedValid=false;}
  return out;
 }
private:
 uint8_t state_=0,cls_=0,id_=0,ckA_=0,ckB_=0,payload_[92]={};
 uint16_t len_=0,idx_=0;M100UbxFix fix_{};
 void check(uint8_t b){ckA_=(uint8_t)(ckA_+b);ckB_=(uint8_t)(ckB_+ckA_);}
 uint32_t u32(int o)const{return (uint32_t)payload_[o]|((uint32_t)payload_[o+1]<<8)|((uint32_t)payload_[o+2]<<16)|((uint32_t)payload_[o+3]<<24);}
 int32_t i32(int o)const{return (int32_t)u32(o);}
 void decode(uint32_t now){
  const uint8_t fix=payload_[20],flags=payload_[21],sv=payload_[23];
  const int32_t lon=i32(24),lat=i32(28),groundSpeed=i32(60);
  const uint32_t hacc=u32(40);
  // Require valid 3D fix and reasonable horizontal uncertainty (mm).
  if(fix<3||!(flags&1)||sv<4||lat>900000000||lat< -900000000||
     lon>1800000000||lon< -1800000000||hacc==0||hacc>100000U||groundSpeed<0){
   fix_.positionValid=false;fix_.speedValid=false;fix_.at=now;return;
  }
  fix_.latE7=lat;fix_.lonE7=lon;fix_.accuracyM=hacc/1000.0;
  fix_.speedCmS=(uint16_t)((uint32_t)groundSpeed/10U>65535U?65535U:(uint32_t)groundSpeed/10U);
  fix_.fixType=fix;fix_.satellites=sv;fix_.at=now;
  fix_.positionValid=fix_.speedValid=true;
 }
};
