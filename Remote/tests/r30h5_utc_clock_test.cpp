#include "../source/radio_utc_time_v1.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(){
 using namespace flytec_time;
 const uint64_t sample=1795347296123ULL;
 uint8_t payload[8]{};
 assert(encode(sample,payload));
 uint64_t decoded=0;
 assert(decode(payload,decoded)&&decoded==sample);
 assert(payload[0]==9&&payload[1]==1);
 payload[0]=8;assert(!decode(payload,decoded));
 payload[0]=9;payload[1]=2;assert(!decode(payload,decoded));
 payload[1]=1;payload[6]=0xE8;payload[7]=0x03;
 assert(!decode(payload,decoded));
 assert(!encode(MIN_MS-1,payload));
 assert(!encode(MAX_MS,payload));
 Discipline clock;
 assert(clock.observe(sample,1000));
 assert(!clock.utcNow(1001,decoded));
 assert(clock.observe(sample+5000,6000));
 assert(clock.utcNow(6000,decoded)&&decoded==sample+5000);
 clock.markRtcSet(6000);
 assert(!clock.shouldSetRtc(7000));
 assert(!clock.utcNow(26001,decoded));
 assert(!clock.observe(sample+2000,8000)); // regression must invalidate
 assert(!clock.qualified());
 clock.reset();
 assert(!clock.utcNow(8000,decoded));
 Discipline wrap;
 assert(wrap.observe(sample,0xFFFFFFF0U));
 assert(wrap.observe(sample+50,0x22U));
 assert(wrap.utcNow(0x23U,decoded)&&decoded==sample+51);
 puts("R30H-5 Remote GNSS UTC HMAC-payload clock discipline portable tests passed (NO_RADIO)");
}
