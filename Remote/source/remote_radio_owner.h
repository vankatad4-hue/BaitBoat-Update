#pragma once
#include "remote_i2c_owner.h"
namespace flytec_rtos {
// Application-owner-only mode selection. A failed initialization consumes
// the claim. Retired hardware is never automatically reassigned.
class RemoteRadioOwner {
public:
 enum class Mode:uint8_t {Unclaimed,Legacy,Worker,Retired};
 bool claimLegacy(){return claim(Mode::Legacy);}
 bool claimWorker(bool commissioned){return commissioned&&claim(Mode::Worker);}
 bool legacy()const{return remoteI2cAllowed()&&mode_==Mode::Legacy;}
 bool worker()const{return remoteI2cAllowed()&&mode_==Mode::Worker;}
 void retire(){if(remoteI2cAllowed()&&mode_!=Mode::Unclaimed)mode_=Mode::Retired;}
private:
 bool claim(Mode next){
  if(!remoteI2cAllowed()||mode_!=Mode::Unclaimed)return false;
  mode_=next;return true;
 }
 Mode mode_=Mode::Unclaimed;
};
// Captured at the local PHY boundary, not a synchronized remote clock.
// Limit queue delay before authentication or state/lease updates.
inline bool remoteRxFresh(uint32_t now,uint32_t capturedAt){
 return uint32_t(now-capturedAt)<100U;
}
}
