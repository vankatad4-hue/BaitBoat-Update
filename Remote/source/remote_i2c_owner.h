#pragma once
#include <atomic>
#include <stdint.h>
// Task identity, not merely CPU identity: two tasks on one core can interleave.
namespace flytec_rtos {
class I2cOwnerFence {
public:
 bool bind(uintptr_t token){
  if(!token)return false;
  uintptr_t expected=0;
  return owner_.compare_exchange_strong(expected,token)||expected==token;
 }
 bool allows(uintptr_t token)const{return token!=0&&owner_.load()==token;}
private:
 std::atomic<uintptr_t> owner_{0};
};
}
#ifndef FLYTEC_I2C_OWNER_HOST_ONLY
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
namespace flytec_rtos {
inline I2cOwnerFence& remoteI2cOwner(){static I2cOwnerFence owner;return owner;}
inline uintptr_t remoteI2cTask(){return reinterpret_cast<uintptr_t>(xTaskGetCurrentTaskHandle());}
inline bool bindRemoteI2cOwner(){return remoteI2cOwner().bind(remoteI2cTask());}
inline bool remoteI2cAllowed(){return remoteI2cOwner().allows(remoteI2cTask());}
}
#endif
