#pragma once
#include <stdint.h>
// R30H-9 pure software instrumentation CONTRACT, not a GPIO or IRQ driver.
// A common-clock analyzer cannot observe C++ events without a physically
// inspected logic-only marker interface. No physical output exists here.
namespace flytec_r30h9 {
enum class Endpoint : uint8_t { BOAT=1, REMOTE=2 };
enum class Event : uint8_t {
 REMOTE_STOP_REQUEST=1,
 REMOTE_NRF_TX_ADMITTED=2,
 REMOTE_E22_UART_ADMITTED=3,
 BOAT_AUTH_FRAME=4,
 BOAT_AUTH_STOP=5,
 BOAT_STOP_INTENT=6,
 BOAT_RTH_ENTRY=7,
 BOAT_RTC_SAMPLE=8,
 REMOTE_RTC_SAMPLE=9
};
enum class Evidence : uint8_t {
 SOFTWARE_LOCAL_UNCERTIFIED=0,
 EXISTING_ELECTRICAL_INTERFACE_UNVERIFIED=1,
 EXTERNAL_CLOCK_CAPTURE_REVIEW_REQUIRED=2
};
struct Record {
 uint32_t localMonotonicMs=0;
 uint16_t commandId=0;
 Event event=Event::REMOTE_STOP_REQUEST;
 Evidence evidence=Evidence::SOFTWARE_LOCAL_UNCERTIFIED;
 Endpoint endpoint=Endpoint::BOAT;
};
class LocalRecorder {
public:
 static constexpr uint8_t CAPACITY=16;
 bool push(Endpoint endpoint,Event ev,uint32_t millisNow,uint16_t id=0) {
  if(size_==CAPACITY){++dropped_;return false;}
  records_[tail_]={millisNow,id,ev,Evidence::SOFTWARE_LOCAL_UNCERTIFIED,endpoint};
  tail_=(uint8_t)((tail_+1U)%CAPACITY);++size_;return true;
 }
 bool pop(Record &r){
  if(!size_)return false;
  r=records_[head_];head_=(uint8_t)((head_+1U)%CAPACITY);--size_;
  return true;
 }
 uint8_t size()const{return size_;}
 uint32_t dropped()const{return dropped_;}
 void reset(){head_=tail_=size_=0;dropped_=0;}
private:
 Record records_[CAPACITY]{};
 uint8_t head_=0,tail_=0,size_=0;
 uint32_t dropped_=0;
};
inline bool isBoatReservedForSafety(int gpio) {
 return gpio==1||gpio==2||gpio==41||gpio==42;
}
inline bool isBoatFlashOrDangerous(int gpio) {
 return gpio==0||gpio==3||gpio==19||gpio==20||gpio==35||
        gpio==36||gpio==37||gpio==38||gpio==43||gpio==44||
        gpio==45||gpio==46||gpio==47||gpio==48;
}
inline bool isBoatAllocated(int gpio) {
 switch(gpio) {
 case 4:case 5:case 6:case 7:case 8:case 9:case 10:case 11:
 case 12:case 13:case 14:case 15:case 16:case 17:case 18:
 case 21:case 39:case 40:return true;
 default:return false;
 }
}
// For Waveshare Standard SKU25948, all documented RGB/touch/SD/RS485,
// USB, UART, sensor AD, boot, and module-memory IO are NO-USE here.
// A pin undocumented as an exposed connector is NOT thereby free.
inline bool isRemoteKnownOccupiedOrRestricted(int gpio) {
 switch(gpio) {
 case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:
 case 8:case 9:case 10:case 11:case 12:case 13:case 14:
 case 15:case 16:case 17:case 18:case 19:case 20:case 21:
 case 35:case 36:case 37:case 38:case 39:case 40:case 41:
 case 42:case 43:case 44:case 45:case 46:case 47:case 48:
 return true;
 default:return false;
 }
}
// This is a proposal filter, NEVER an output authorization.
// In the current baseline all diagnostic outputs are explicitly barred.
inline bool currentlyApprovedOutput(Endpoint, int) {return false;}
struct PhysicalGate {
 bool boardRevisionInspected=false;
 bool actualHeaderContinuityMeasured=false;
 bool pinConflictAuditSigned=false;
 bool ioVoltageMeasured=false;
 bool actuatorSupplyVmVhDisconnected=false;
 bool radioAntennaAndBandApproved=false;
 bool trueExternalCommonClockPresent=false;
 bool eventToEdgeLatencyCharacterized=false;
 bool independentReviewerSigned=false;
};
inline bool prerequisitesDocumented(const PhysicalGate &g) {
 return g.boardRevisionInspected&&g.actualHeaderContinuityMeasured&&
  g.pinConflictAuditSigned&&g.ioVoltageMeasured&&
  g.actuatorSupplyVmVhDisconnected&&g.radioAntennaAndBandApproved&&
  g.trueExternalCommonClockPresent&&g.eventToEdgeLatencyCharacterized&&
  g.independentReviewerSigned;
}
} // namespace flytec_r30h9
