#define FLYTEC_I2C_OWNER_HOST_ONLY
#include "remote_i2c_owner.h"
#include <cassert>
#include <thread>
int main(){
 flytec_rtos::I2cOwnerFence fence;
 assert(!fence.allows(1)&&!fence.bind(0));
 assert(fence.bind(1)&&fence.bind(1)&&fence.allows(1));
 std::thread other([&]{assert(!fence.bind(2)&&!fence.allows(2));});
 other.join();
 assert(fence.allows(1)&&!fence.allows(0));
}
