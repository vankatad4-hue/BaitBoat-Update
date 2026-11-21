#include <cassert>
#include "../source/heltec_sx1262_airtime.h"
int main(){
 assert(sx1262AirtimeMs(32,9,125000,1)==247U);
 assert(sx1262AirtimeMs(32,7,125000,1)==72U);
 assert(sx1262AirtimeMs(32,6,125000,1)==0U);
 Sx1262DutyLimiter b;
 assert(b.reserve(1000U,247U));
 assert(!b.allows(2000U,247U));
 assert(!b.reserve(3000U,247U));
 assert(b.reserve(25700U,247U));
 assert(!b.allows(25900U,247U));
}
