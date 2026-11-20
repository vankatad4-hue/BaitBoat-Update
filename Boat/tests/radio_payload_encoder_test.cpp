#include "../source/radio_payload_encoder.h"
#include <assert.h>

int main() {
    uint8_t p[8]; RadioDecodedPayload d{};
    assert(radio_encodeHeartbeatPayload(0x0F,3,p));
    assert(radio_decodeHeartbeatPayload(p,d) && d.heartbeatFlags==0x0F && d.buildMajor==3);
    assert(!radio_encodeHeartbeatPayload(0x10,3,p));

    assert(radio_encodeAckPayload(RADIO_ACK_ACCEPTED,7,0x1234,1,0x05,p));
    assert(radio_decodeAckPayload(p,d) && d.commandId==0x1234 && d.commandType==7 && d.interlockFlags==5);
    assert(!radio_encodeAckPayload(0,7,0,0,0,p));
    assert(!radio_encodeAckPayload(0,11,1,0,0,p));
    assert(!radio_encodeAckPayload(8,7,1,0,0,p));
    assert(!radio_encodeAckPayload(0,7,1,3,0,p));
    assert(!radio_encodeAckPayload(0,7,1,0,0x80,p));

    d={}; d.kind=RADIO_PAYLOAD_STATUS; d.boatMode=2; d.stopState=1; d.safetyFlags=0x12; d.batteryPercent=80; d.gpsFixType=3; d.satellites=9;
    assert(radio_encodeTelemetryPayload(d,p) && radio_decodeTelemetryPayload(p,d));
    assert(d.boatMode==2 && d.batteryPercent==80 && d.satellites==9);
    d.boatMode=5; assert(!radio_encodeTelemetryPayload(d,p));

    d={}; d.kind=RADIO_PAYLOAD_BATTERY_SPEED; d.batteryMillivolts=12100; d.speedCmPerSecond=250;
    assert(radio_encodeTelemetryPayload(d,p) && radio_decodeTelemetryPayload(p,d));
    assert(d.batteryMillivolts==12100 && d.speedCmPerSecond==250);
    d.speedCmPerSecond=10001; assert(!radio_encodeTelemetryPayload(d,p));

    d={}; d.kind=RADIO_PAYLOAD_GNSS_LATITUDE; d.coordinateSample=44; d.coordinateE7=420000000; d.horizontalAccuracyCm=125;
    assert(radio_encodeTelemetryPayload(d,p) && radio_decodeTelemetryPayload(p,d));
    assert(d.coordinateE7==420000000 && d.horizontalAccuracyCm==125);
    d.coordinateE7=900000001; assert(!radio_encodeTelemetryPayload(d,p));

    d={}; d.kind=RADIO_PAYLOAD_GNSS_LONGITUDE; d.coordinateSample=44; d.coordinateE7=250000000;
    assert(radio_encodeTelemetryPayload(d,p) && radio_decodeTelemetryPayload(p,d));
    assert(d.coordinateE7==250000000 && d.coordinateSample==44);
    d.coordinateE7=1800000001; assert(!radio_encodeTelemetryPayload(d,p));

    d={}; d.kind=RADIO_PAYLOAD_NAVIGATION; d.headingCentidegrees=18000; d.speedCmPerSecond=100; d.navigationStatus=2; d.waypointIndex=3; d.navigationSample=77;
    assert(radio_encodeTelemetryPayload(d,p) && radio_decodeTelemetryPayload(p,d));
    assert(d.headingCentidegrees==18000 && d.waypointIndex==3);
    d.headingCentidegrees=36000; assert(!radio_encodeTelemetryPayload(d,p));

    d={}; d.kind=RADIO_PAYLOAD_SONAR_SUMMARY; d.depthCm=250; d.sonarRangeCm=1200; d.sonarHealth=1; d.sonarAlarmFlags=3; d.sonarSample=5;
    assert(radio_encodeTelemetryPayload(d,p) && radio_decodeTelemetryPayload(p,d));
    assert(d.depthCm==250 && d.sonarAlarmFlags==3);
    d.sonarHealth=4; assert(!radio_encodeTelemetryPayload(d,p));

    d={}; d.kind=RADIO_PAYLOAD_INVALID; assert(!radio_encodeTelemetryPayload(d,p));
    return 0;
}
