#pragma once
#include <stdint.h>
#include <string.h>
namespace flytec_r30h123 {
// Latest validated snapshots only. No STOP STATUS or diagnostic bypass.
// Latitude/longitude are paired by sample ID AND snapshot time. If a new
// pair replaces an incomplete transmission, abandon the older remainder.
class TelemetrySlots {
 struct Slot {uint8_t bytes[8]{};uint32_t at=0;bool valid=false;};
 Slot slots_[6]{};
 uint8_t cursor_=0;
 bool coordinateHalfTaken_=false;
 uint8_t coordinateId_=0;
 uint32_t coordinateAt_=0;
 bool matches(const Slot& a,const Slot& b)const{
  return a.valid&&b.valid&&a.at==b.at&&a.bytes[1]==b.bytes[1];
 }
public:
 static constexpr uint32_t MAX_AGE_MS=250U;
 void reset(){for(auto &s:slots_)s.valid=false;cursor_=0;coordinateHalfTaken_=false;}
 bool offer(const uint8_t p[8],uint32_t at){
  if(!p||p[0]<2||p[0]>7)return false;
  const unsigned idx=p[0]-2;
  if(idx==1||idx==2){
   if(coordinateHalfTaken_&&(coordinateAt_!=at||coordinateId_!=p[1])){
    slots_[1].valid=slots_[2].valid=false;coordinateHalfTaken_=false;
   }
   auto &other=slots_[idx==1?2:1];
   if(other.valid&&(other.at!=at||other.bytes[1]!=p[1]))other.valid=false;
  }
  auto &s=slots_[idx];memcpy(s.bytes,p,8);s.at=at;s.valid=true;return true;
 }
 bool take(uint32_t now,uint8_t p[8],uint32_t &remaining){
  if(!p)return false;
  for(unsigned i=0;i<6;++i){
   const unsigned idx=(cursor_+i)%6;auto &s=slots_[idx];
   if(!s.valid)continue;
   const uint32_t age=uint32_t(now-s.at);
   if(age>=MAX_AGE_MS){s.valid=false;if(idx==1||idx==2)coordinateHalfTaken_=false;continue;}
   if(idx==1||idx==2){
    const auto &other=slots_[idx==1?2:1];
    const bool remainder=coordinateHalfTaken_&&coordinateAt_==s.at&&coordinateId_==s.bytes[1];
    if(!remainder&&!matches(s,other))continue;
    coordinateHalfTaken_=!remainder;coordinateAt_=s.at;coordinateId_=s.bytes[1];
   }
   s.valid=false;memcpy(p,s.bytes,8);remaining=MAX_AGE_MS-age;cursor_=(idx+1)%6;return true;
  }
  return false;
 }
};
}
