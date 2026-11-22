#include "radio_payload_codec.h"
#include <assert.h>
#include <stdint.h>

int main() {
    RadioDecodedPayload d{};
    uint8_t p[8]={0};
    p[0]=0x0F; p[1]=3;
    assert(radio_decodeHeartbeatPayload(p,d) && d.kind==RADIO_PAYLOAD_HEARTBEAT && d.heartbeatFlags==15);
    p[0]=0x10; assert(!radio_decodeHeartbeatPayload(p,d) && d.kind==RADIO_PAYLOAD_INVALID);
    p[0]=0; p[1]=1; p[2]=9; p[3]=0x34; p[4]=0x12; p[5]=0; p[6]=0x0F; p[7]=0;
    assert(radio_decodeAckPayload(p,d) && d.commandId==0x1234 && d.ackResult==1);
    p[7]=1; assert(!radio_decodeAckPayload(p,d) && d.kind==RADIO_PAYLOAD_INVALID);
    p[0]=1; p[1]=2; p[2]=0; p[3]=0x7F; p[4]=55; p[5]=3; p[6]=12; p[7]=0;
    assert(radio_decodeTelemetryPayload(p,d) && d.kind==RADIO_PAYLOAD_STATUS && d.batteryPercent==55);
    p[3]=0x80; assert(radio_decodeTelemetryPayload(p,d) && d.kind==RADIO_PAYLOAD_STATUS && d.safetyFlags==0x80);
    p[0]=2; p[1]=0xB8; p[2]=0x0B; p[3]=0x64; p[4]=0; p[5]=p[6]=p[7]=0;
    assert(radio_decodeTelemetryPayload(p,d) && d.batteryMillivolts==3000 && d.speedCmPerSecond==100);
    p[0]=3; p[1]=9; p[2]=0x00; p[3]=0x65; p[4]=0xCD; p[5]=0x1D; p[6]=0xE7; p[7]=3;
    assert(radio_decodeTelemetryPayload(p,d) && d.coordinateSample==9 && d.coordinateE7==500000000L);
    p[0]=4; p[1]=9; p[2]=0x00; p[3]=0xCA; p[4]=0x9A; p[5]=0x3B; p[6]=0; p[7]=0;
    assert(radio_decodeTelemetryPayload(p,d) && d.coordinateE7==1000000000L);
    p[0]=5; p[1]=0x10; p[2]=0x27; p[3]=0x64; p[4]=0; p[5]=1; p[6]=4; p[7]=8;
    assert(radio_decodeTelemetryPayload(p,d) && d.headingCentidegrees==10000 && d.navigationStatus==1);
    p[0]=6; p[1]=0x2C; p[2]=1; p[3]=0xE8; p[4]=3; p[5]=1; p[6]=1; p[7]=5;
    assert(radio_decodeTelemetryPayload(p,d) && d.kind==RADIO_PAYLOAD_SONAR_SUMMARY && d.depthCm==300);
    p[6]=0x80; assert(!radio_decodeTelemetryPayload(p,d) && d.kind==RADIO_PAYLOAD_INVALID);
    p[0]=7; assert(!radio_decodeTelemetryPayload(p,d) && d.kind==RADIO_PAYLOAD_INVALID);
    // Latitude is 91.0 degrees E7 (910,000,000), outside valid range.
    p[0]=3; p[1]=1; p[2]=0x80; p[3]=0x7F; p[4]=0x3D; p[5]=0x36; p[6]=0; p[7]=0;
    assert(!radio_decodeTelemetryPayload(p,d) && d.kind==RADIO_PAYLOAD_INVALID && d.coordinateE7==0);
    p[0]=5; p[1]=0x40; p[2]=0x9C; p[3]=0; p[4]=0; p[5]=1; p[6]=0; p[7]=0;
    assert(!radio_decodeTelemetryPayload(p,d) && d.kind==RADIO_PAYLOAD_INVALID);
    return 0;
}
