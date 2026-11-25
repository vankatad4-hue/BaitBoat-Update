#include "../source/utc_clock_ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(){
 using namespace flytec_r30h6;
 char label[48]{};
 ClockReport invalid{};
 assert(!flytec_r30h7::formatUtc(invalid,label,sizeof(label)));
 assert(!strcmp(label,"GPS UTC: --:--:--"));
 ClockReport coarse{};
 coarse.quality=ClockQuality::AUTHENTICATED_REMOTE_COARSE;
 coarse.unixMs=1795347296123ULL;
 coarse.ageMs=19999U;
 assert(flytec_r30h7::formatUtc(coarse,label,sizeof(label)));
 assert(!strncmp(label,"GPS UTC: ",9));
 assert(strstr(label," ~")!=nullptr);
 assert(strstr(label,"--:--:--")==nullptr);
 char tiny[2]{};
 assert(!flytec_r30h7::formatUtc(coarse,tiny,sizeof(tiny)));
 assert(tiny[1]==0);
 coarse.ageMs=20001U;
 assert(!flytec_r30h7::formatUtc(coarse,label,sizeof(label)));
 assert(!strcmp(label,"GPS UTC: --:--:--"));
 coarse.ageMs=0;
 coarse.traceReferenceCertified=true;
 assert(!flytec_r30h7::formatUtc(coarse,label,sizeof(label)));
 coarse.traceReferenceCertified=false;
 coarse.rfEndToEndStopClock=true;
 assert(!flytec_r30h7::formatUtc(coarse,label,sizeof(label)));
 coarse.rfEndToEndStopClock=false;
 coarse.quality=ClockQuality::RECEIVER_COARSE; // Boat's local quality not Remote quality
 assert(!flytec_r30h7::formatUtc(coarse,label,sizeof(label)));
 coarse.quality=ClockQuality::AUTHENTICATED_REMOTE_COARSE;
 coarse.unixMs=flytec_time::MIN_MS-1;
 assert(!flytec_r30h7::formatUtc(coarse,label,sizeof(label)));
 printf("R30H-7 Remote UI UTC placeholder/coarse/stale quality guard passed (NO_RF)\n");
}
