#include "radio_payload_codec.h"
#include "radio_safety_diagnostic_v1.h"
#include "sonar_echo_profile_v1.h"
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
    if(p[0]!=0 || p[1]>RADIO_ACK_INTERNAL_FAILURE || p[2]<1 || p[2]>11 ||
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
    case 10:
        if(!reservedZero(p,4,7)){resetOut(out);return false;}
        out.kind=RADIO_PAYLOAD_WATER_TEMPERATURE;
        out.temperatureCentiC=(int16_t)readU16LE(p+1);out.waterTemperatureSample=p[3];
        if(out.temperatureCentiC < -500 || out.temperatureCentiC > 5000){resetOut(out);return false;}
        return true;
    case sonar_profile_v1::SUBTYPE: {
        sonar_profile_v1::Fragment fragment{};
        if(!sonar_profile_v1::decode(p,fragment)){resetOut(out);return false;}
        out.kind=RADIO_PAYLOAD_SONAR_PROFILE_FRAGMENT;
        out.sonarProfileId=fragment.profileId;
        out.sonarProfileChunk=fragment.chunkIndex;
        memcpy(out.sonarProfileBins,fragment.bins,sizeof(out.sonarProfileBins));
        return true;
    }
    default: resetOut(out); return false;
    }
}

static void clearPayload(uint8_t out[8]){for(uint8_t i=0;i<8;i++)out[i]=0;}
static void writeU16LE(uint8_t*p,uint16_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void writeI32LE(uint8_t*p,int32_t v){uint32_t u;memcpy(&u,&v,sizeof(u));p[0]=(uint8_t)u;p[1]=(uint8_t)(u>>8);p[2]=(uint8_t)(u>>16);p[3]=(uint8_t)(u>>24);}
bool radio_encodeHeartbeatPayload(uint8_t flags,uint8_t build,uint8_t out[8]){clearPayload(out);if((flags&0xF0U)!=0)return false;out[0]=flags;out[1]=build;return true;}
bool radio_encodeAckPayload(uint8_t result,uint8_t type,uint16_t id,uint8_t stop,uint8_t interlocks,uint8_t out[8]){clearPayload(out);if(result>RADIO_ACK_INTERNAL_FAILURE||type<1||type>11||id==0||stop>2||(interlocks&0xF0U)!=0)return false;out[0]=0;out[1]=result;out[2]=type;writeU16LE(out+3,id);out[5]=stop;out[6]=interlocks;return true;}
bool radio_encodeTelemetryStatusPayload(uint8_t mode,uint8_t stop,uint8_t safety,uint8_t percent,uint8_t fix,uint8_t sats,uint8_t out[8]){clearPayload(out);if(mode>4||stop>2||(safety&0x80U)!=0||(percent>100&&percent!=255))return false;out[0]=1;out[1]=mode;out[2]=stop;out[3]=safety;out[4]=percent;out[5]=fix;out[6]=sats;return true;}
bool radio_encodeBatterySpeedPayload(uint16_t mv,uint16_t speed,uint8_t out[8]){clearPayload(out);if(speed!=0xFFFFU&&speed>10000U)return false;out[0]=2;writeU16LE(out+1,mv);writeU16LE(out+3,speed);return true;}
bool radio_encodeLatitudePayload(uint8_t sample,int32_t lat,uint16_t accuracy,uint8_t out[8]){clearPayload(out);if(lat < -900000000L||lat>900000000L)return false;out[0]=3;out[1]=sample;writeI32LE(out+2,lat);writeU16LE(out+6,accuracy);return true;}
bool radio_encodeLongitudePayload(uint8_t sample,int32_t lon,uint8_t out[8]){clearPayload(out);if(lon < -1800000000L||lon>1800000000L)return false;out[0]=4;out[1]=sample;writeI32LE(out+2,lon);return true;}
bool radio_encodeNavigationPayload(uint16_t heading,uint16_t speed,uint8_t status,uint8_t waypoint,uint8_t sample,uint8_t out[8]){clearPayload(out);if((heading!=0xFFFFU&&heading>35999U)||(speed!=0xFFFFU&&speed>10000U)||status>5)return false;out[0]=5;writeU16LE(out+1,heading);writeU16LE(out+3,speed);out[5]=status;out[6]=waypoint;out[7]=sample;return true;}
bool radio_encodeSonarSummaryPayload(uint16_t depth,uint16_t range,uint8_t health,uint8_t alarms,uint8_t sample,uint8_t out[8]){clearPayload(out);if(health>3||(alarms&0xF0U)!=0)return false;out[0]=6;writeU16LE(out+1,depth);writeU16LE(out+3,range);out[5]=health;out[6]=alarms;out[7]=sample;return true;}
