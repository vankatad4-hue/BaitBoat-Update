#include "radio_payload_codec.h"
#include "radio_safety_diagnostic_v1.h"
#include <string.h>

static uint16_t readU16LE(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
static int32_t readI32LE(const uint8_t *p) {
    const uint32_t u=(uint32_t)p[0] | ((uint32_t)p[1]<<8) |
                     ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
    int32_t value;
    memcpy(&value,&u,sizeof(value));
    return value;
}
static void resetOut(RadioDecodedPayload &out) {
    out={}; out.kind=RADIO_PAYLOAD_INVALID;
}
static bool reservedZero(const uint8_t *p, uint8_t from, uint8_t toInclusive) {
    for(uint8_t i=from;i<=toInclusive;i++) if(p[i]!=0) return false;
    return true;
}

bool radio_decodeHeartbeatPayload(const uint8_t p[8], RadioDecodedPayload &out) {
    resetOut(out);
    if((p[0]&0xF0U)!=0 || !reservedZero(p,2,7)) return false;
    out.kind=RADIO_PAYLOAD_HEARTBEAT; out.heartbeatFlags=p[0]; out.buildMajor=p[1];
    return true;
}
bool radio_decodeAckPayload(const uint8_t p[8], RadioDecodedPayload &out) {
    resetOut(out);
    if(p[0]!=0 || p[1]>RADIO_ACK_INTERNAL_FAILURE || p[2]<1 || p[2]>10 ||
       p[5]>2 || (p[6]&0xF0U)!=0 || p[7]!=0) return false;
    out.kind=RADIO_PAYLOAD_ACK; out.subtype=0; out.ackResult=p[1]; out.commandType=p[2];
    out.commandId=readU16LE(p+3); out.stopState=p[5]; out.interlockFlags=p[6];
    return true;
}
bool radio_decodeTelemetryPayload(const uint8_t p[8], RadioDecodedPayload &out) {
    resetOut(out); out.subtype=p[0];
    switch(p[0]) {
    case 1:
        if(p[1]>4 || p[2]>2 || (p[4]>100 && p[4]!=255) || p[7]!=0) { resetOut(out); return false; }
        out.kind=RADIO_PAYLOAD_STATUS; out.boatMode=p[1]; out.stopState=p[2]; out.safetyFlags=p[3];
        out.batteryPercent=p[4]; out.gpsFixType=p[5]; out.satellites=p[6]; return true;
    case 2:
        if(!reservedZero(p,5,7)) { resetOut(out); return false; }
        out.kind=RADIO_PAYLOAD_BATTERY_SPEED; out.batteryMillivolts=readU16LE(p+1); out.speedCmPerSecond=readU16LE(p+3);
        if(out.speedCmPerSecond!=0xFFFFU && out.speedCmPerSecond>10000U) { resetOut(out); return false; }
        return true;
    case 3:
        out.kind=RADIO_PAYLOAD_GNSS_LATITUDE; out.coordinateSample=p[1]; out.coordinateE7=readI32LE(p+2); out.horizontalAccuracyCm=readU16LE(p+6);
        if(out.coordinateE7 < -900000000L || out.coordinateE7 > 900000000L) { resetOut(out); return false; }
        return true;
    case 4:
        if(!reservedZero(p,6,7)) { resetOut(out); return false; }
        out.kind=RADIO_PAYLOAD_GNSS_LONGITUDE; out.coordinateSample=p[1]; out.coordinateE7=readI32LE(p+2);
        if(out.coordinateE7 < -1800000000L || out.coordinateE7 > 1800000000L) { resetOut(out); return false; }
        return true;
    case 5:
        out.kind=RADIO_PAYLOAD_NAVIGATION; out.headingCentidegrees=readU16LE(p+1); out.speedCmPerSecond=readU16LE(p+3);
        out.navigationStatus=p[5]; out.waypointIndex=p[6]; out.navigationSample=p[7];
        if((out.headingCentidegrees!=0xFFFFU && out.headingCentidegrees>35999U) ||
           (out.speedCmPerSecond!=0xFFFFU && out.speedCmPerSecond>10000U) || out.navigationStatus>5) { resetOut(out); return false; }
        return true;
    case 6:
        out.kind=RADIO_PAYLOAD_SONAR_SUMMARY; out.depthCm=readU16LE(p+1); out.sonarRangeCm=readU16LE(p+3);
        out.sonarHealth=p[5]; out.sonarAlarmFlags=p[6]; out.sonarSample=p[7];
        if(out.sonarHealth>3 || (out.sonarAlarmFlags&0xF0U)!=0) { resetOut(out); return false; }
        return true;
    case 7:
        if(p[7]!=0){resetOut(out);return false;}
        out.kind=RADIO_PAYLOAD_WEATHER;
        out.temperatureCentiC=(int16_t)readU16LE(p+1);
        out.pressurePa=(uint32_t)p[3]|((uint32_t)p[4]<<8)|((uint32_t)p[5]<<16)|((uint32_t)p[6]<<24);
        if(out.temperatureCentiC < -4000 || out.temperatureCentiC > 8500 || out.pressurePa < 30000 || out.pressurePa > 120000){resetOut(out);return false;}
        return true;
    case 8: {
        // Optional R30F diagnostic. Authenticated RadioFrame checks belong to
        // the transport; observing this subtype NEVER confirms STOP or motor
        // readiness. Reject older-version or falsely commissioned payloads.
        radio_r30f::Diagnostic diagnostic{};
        if(!radio_r30f::decode(p,diagnostic)) {resetOut(out);return false;}
        out.kind=RADIO_PAYLOAD_SAFETY_DIAGNOSTIC;
        out.diagnosticReason=diagnostic.reason;
        out.diagnosticFlags=diagnostic.flags;
        out.diagnosticMillivolts=diagnostic.millivolts;
        out.diagnosticSequence=diagnostic.sequence;
        return true;
    }
    default: resetOut(out); return false;
    }
}
