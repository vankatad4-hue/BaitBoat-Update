#include "i2c_bus_guard.h"
#include <cassert>
#include <thread>
int main(){
 flytec_rtos::I2cBusGuard bus;
 {flytec_rtos::I2cBusGuard::Lease l(bus);assert(!l);}
 assert(bus.begin());assert(bus.begin());
 {flytec_rtos::I2cBusGuard::Lease l(bus);assert(l);
  std::thread contender([&]{flytec_rtos::I2cBusGuard::Lease other(bus);assert(!other);});
  contender.join();
 }
 {flytec_rtos::I2cBusGuard::Lease l(bus);assert(l);}
}
