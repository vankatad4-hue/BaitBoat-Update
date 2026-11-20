#include "radio_payload_codec.h"

static uint16_t readU16LE(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
static int32_t readI32LE(const uint8_t *p) {
    const uint32_t u=(uint32_t)p[0] | ((uint32_t)p[1]<<8) |
                     ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
    int32_t value;
    // memcpy avoids implementation-defined unsigned-to-signed conversion.
    __builtin_memcpy(&value,&u,sizeof(value));
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
    out.kind=RADIO_PAYLOAD_HEARTBEAT;
    out.heartbeatFlags=p[0]; out.buildMajor=p[1];
    return true;
}

bool radio_decodeAckPayload(const uint8_t p[8], RadioDecodedPayload &out) {
    resetOut(out);
    if(p[0]!=0 || p[1]>RADIO_ACK_INTERNAL_FAILURE ||
       p[2]<1 || p[2]>10 || p[5]>2 || (p[6]&0xF0U)!=0 || p[7]!=0) return false;
    out.kind=RADIO_PAYLOAD_ACK; out.subtype=0;
    out.ackResult=p[1]; out.commandType=p[2]; out.commandId=readU16LE(p+3);
    out.stopState=p[5]; out.interlockFlags=p[6];
    return true;
}

bool radio_decodeTelemetryPayload(const uint8_t p[8], RadioDecodedPayload &out) {
    resetOut(out);
    out.subtype=p[0];
    switch(p[0]) {
    case 1: // Boat mode, STOP and safety summary
        if(p[1]>4 || p[2]>2 || (p[3]&0x80U)!=0 ||
           (p[4]>100 && p[4]!=255) || p[7]!=0) return false;
        out.kind=RADIO_PAYLOAD_STATUS; out.boatMode=p[1]; out.stopState=p[2];
        out.safetyFlags=p[3]; out.batteryPercent=p[4]; out.gpsFixType=p[5]; out.satellites=p[6];
        return true;
    case 2: // Battery mV + speed
        if(!reservedZero(p,5,7)) return false;
        out.kind=RADIO_PAYLOAD_BATTERY_SPEED; out.batteryMillivolts=readU16LE(p+1);
        out.speedCmPerSecond=readU16LE(p+3);
        if(out.speedCmPerSecond!=0xFFFFU && out.speedCmPerSecond>10000U) { resetOut(out); return false; }
        return true;
    case 3: // Latitude E7 + accuracy cm
        out.kind=RADIO_PAYLOAD_GNSS_LATITUDE; out.coordinateSample=p[1];
        out.coordinateE7=readI32LE(p+2); out.horizontalAccuracyCm=readU16LE(p+6);
        if(out.coordinateE7 < -900000000L || out.coordinateE7 > 900000000L) { resetOut(out); return false; }
        return true;
    case 4: // Longitude E7
        if(!reservedZero(p,6,7)) return false;
        out.kind=RADIO_PAYLOAD_GNSS_LONGITUDE; out.coordinateSample=p[1];
        out.coordinateE7=readI32LE(p+2);
        if(out.coordinateE7 < -1800000000L || out.coordinateE7 > 1800000000L) { resetOut(out); return false; }
        return true;
    case 5: // Heading/navigation
        out.kind=RADIO_PAYLOAD_NAVIGATION; out.headingCentidegrees=readU16LE(p+1);
        out.speedCmPerSecond=readU16LE(p+3); out.navigationStatus=p[5];
        out.waypointIndex=p[6]; out.navigationSample=p[7];
        if((out.headingCentidegrees!=0xFFFFU && out.headingCentidegrees>35999U) ||
           (out.speedCmPerSecond!=0xFFFFU && out.speedCmPerSecond>10000U) || out.navigationStatus>5) {
            resetOut(out); return false;
        }
        return true;
    case 6: // Sonar summary; this is not raw echogram data
        out.kind=RADIO_PAYLOAD_SONAR_SUMMARY; out.depthCm=readU16LE(p+1);
        out.sonarRangeCm=readU16LE(p+3); out.sonarHealth=p[5];
        out.sonarAlarmFlags=p[6]; out.sonarSample=p[7];
        if(out.sonarHealth>3 || (out.sonarAlarmFlags&0xF0U)!=0) { resetOut(out); return false; }
        return true;
    default:
        resetOut(out); return false;
    }
}
