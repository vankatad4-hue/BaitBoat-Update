#pragma once
#include <stdint.h>
#include <string.h>
namespace flytec_r30h123 {
// Best-effort telemetry, owner-only. Six latest-value slots for wire
// subtypes 2..7. No STOP STATUS or diagnostic bypass, no unbounded FIFO.
// Timestamp is the validated snapshot creation time, not GNSS measurement time.
class TelemetrySlots {
 struct Slot {uint8_t bytes[8]{};uint32_t at=0;bool valid=false;};
 Slot slots_[6]{};
 uint8_t cursor_=0;
public:
 static constexpr uint32_t MAX_AGE_MS=250U;
 void reset(){for(auto &s:slots_)s.valid=false;cursor_=0;}
 bool offer(const uint8_t p[8],uint32_t at){
  if(!p||p[0]<2||p[0]>7)return false;
  auto &s=slots_[p[0]-2];memcpy(s.bytes,p,8);s.at=at;s.valid=true;return true;
 }
 bool take(uint32_t now,uint8_t p[8],uint32_t &remaining){
  if(!p)return false;
  for(unsigned i=0;i<6;++i){
   const unsigned idx=(cursor_+i)%6;auto &s=slots_[idx];
   if(!s.valid)continue;
   const uint32_t age=uint32_t(now-s.at);
   s.valid=false;
   if(age>=MAX_AGE_MS)continue;
   memcpy(p,s.bytes,8);remaining=MAX_AGE_MS-age;cursor_=(idx+1)%6;return true;
  }
  return false;
 }
};
}
