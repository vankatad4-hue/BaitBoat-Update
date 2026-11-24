#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
// R30H-98: opt-in arbitration primitive; not yet connected to Wire clients.
// Initialize before workers start; destroy only after all clients join.
// Guard spans an entire logical transaction, including repeated START.
namespace flytec_rtos {
class I2cBusGuard {
public:
 I2cBusGuard()=default;
 I2cBusGuard(const I2cBusGuard&)=delete;
 I2cBusGuard& operator=(const I2cBusGuard&)=delete;
 bool begin(){if(!mutex_)mutex_=xSemaphoreCreateMutex();return mutex_!=nullptr;}
 ~I2cBusGuard(){if(mutex_)vSemaphoreDelete(mutex_);}
 class Lease {
 public:
  explicit Lease(I2cBusGuard& bus):mutex_(bus.mutex_),held_(mutex_&&xSemaphoreTake(mutex_,0)==pdTRUE){}
  ~Lease(){if(held_)xSemaphoreGive(mutex_);}
  Lease(const Lease&)=delete;
  Lease& operator=(const Lease&)=delete;
  explicit operator bool()const{return held_;}
 private:
  SemaphoreHandle_t mutex_;
  bool held_;
 };
private:
 SemaphoreHandle_t mutex_=nullptr;
};
}
