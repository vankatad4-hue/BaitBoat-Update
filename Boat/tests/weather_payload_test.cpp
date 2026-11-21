#include <cassert>
#include <cstdint>
#include "../source/radio_payload_codec.h"
int main(){
 uint8_t b[8]={7,0xC4,0x09,0x65,0x8A,0x01,0,0};
 RadioDecodedPayload d{};
 assert(radio_decodeTelemetryPayload(b,d));
 assert(d.kind==RADIO_PAYLOAD_WEATHER);
 assert(d.temperatureCentiC==2500);
 assert(d.pressurePa==100965);
 b[7]=1;assert(!radio_decodeTelemetryPayload(b,d));
 b[7]=0;b[3]=0; b[4]=0;b[5]=0;b[6]=0;
 assert(!radio_decodeTelemetryPayload(b,d));
}
