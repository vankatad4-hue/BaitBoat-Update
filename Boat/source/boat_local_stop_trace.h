#pragma once
#include <stdint.h>

// R30H-2: fixed-size diagnostic-only recorder. Never controls/acknowledges any
// motor, release coil, STOP latch, radio session or radio freshness.
// The recorded times are BOAT-LOCAL millis(), not cross-device timestamps.
// Omission/overflow invalidates completeness of an event capture.
namespace boat_r30h2 {
enum Event : uint8_t {
 SESSION_ESTABLISHED=1,
 AUTH_PRIMARY_SAMPLE=2,
 AUTH_BACKUP_SAMPLE=3,
 BOTH_LINKS_STALE=4,
 AUTH_LINK_RESTORED=5,
 STOP_AUTH_RECEIVED=6,
 STOP_LOGICAL_INTENT=7,
 RTH_LOCAL_ENTRY=8
};
struct Record {
 uint32_t timestampMs;
 uint32_t lastAcceptedRxMs;
 uint16_t commandId;
 uint8_t event;
 uint8_t link; // 0=none;1=nRF;2=E22
 uint8_t freshMask; // bit0=nRF;bit1=E22
};
class LocalTrace {
public:
 static constexpr uint8_t CAPACITY=32;
 void push(Event event,uint32_t now,uint32_t lastAccepted,uint8_t link,
           uint8_t freshMask,uint16_t commandId=0){
  if(size_>=CAPACITY){++overflows_;return;} // no overwrite of safety sequence
  queue_[tail_]={now,lastAccepted,commandId,(uint8_t)event,link,freshMask};
  tail_=(uint8_t)((tail_+1U)%CAPACITY);
  ++size_;
 }
 bool take(Record &r){
  if(!size_)return false;
  r=queue_[head_];head_=(uint8_t)((head_+1U)%CAPACITY);--size_;return true;
 }
 uint8_t size()const{return size_;}
 uint32_t overflows()const{return overflows_;}
 void reset(){head_=tail_=size_=0;overflows_=0;}
private:
 Record queue_[CAPACITY]{};
 uint8_t head_=0,tail_=0,size_=0;
 uint32_t overflows_=0;
};
inline const char *label(uint8_t e){
 switch(e){
 case SESSION_ESTABLISHED:return "SESSION_ESTABLISHED";
 case AUTH_PRIMARY_SAMPLE:return "AUTH_PRIMARY_SAMPLE";
 case AUTH_BACKUP_SAMPLE:return "AUTH_BACKUP_SAMPLE";
 case BOTH_LINKS_STALE:return "BOTH_LINKS_STALE";
 case AUTH_LINK_RESTORED:return "AUTH_LINK_RESTORED";
 case STOP_AUTH_RECEIVED:return "STOP_AUTH_RECEIVED";
 case STOP_LOGICAL_INTENT:return "STOP_LOGICAL_INTENT";
 case RTH_LOCAL_ENTRY:return "RTH_LOCAL_ENTRY";
 default:return "INVALID_EVENT";
 }
}
} // namespace boat_r30h2
