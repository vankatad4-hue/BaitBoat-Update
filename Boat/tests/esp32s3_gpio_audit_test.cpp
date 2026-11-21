#include <cassert>
#include "../source/esp32s3_gpio_audit.h"
int main(){
 for(int n:{0,3,19,20,35,36,37,43,44,45,46,47,48})
  assert(!boatEsp32s3PinSafe3v3((int8_t)n));
 for(int n:{2,4,5,7,8,9,10,11,12,13,14,15,16,17,18,21,39,40,41,42})
  assert(boatEsp32s3PinSafe3v3((int8_t)n));
 Mdd20aDirectProfile p{true,39,40,41,42,2,21};
 uint64_t allowed=(1ULL<<39)|(1ULL<<40)|(1ULL<<41)|(1ULL<<42)|(1ULL<<2)|(1ULL<<21);
 assert(boatEsp32s3MotorPinAudit(p,allowed));
 p.pwmLeft=47;allowed|=1ULL<<47;
 assert(!boatEsp32s3MotorPinAudit(p,allowed));
 p.pwmLeft=39;p.watchdogEdge=39;
 assert(!boatEsp32s3MotorPinAudit(p,allowed));
}
