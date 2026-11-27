#include "radio_payload_encoder.h"
#include "radio_safety_diagnostic_v1.h"
#include <string.h>
static void clear8(uint8_t out[8]){memset(out,0,8);}
static void put16(uint8_t*p,uint16_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void put32(uint8_t*p,int32_t v){const uint32_t u=(uint32_t)v;p[0]=(uint8_t)u;p[1]=(uint8_t)(u>>8);p[2]=(uint8_t)(u>>16);p[3]=(uint8_t)(u>>24);}
bool radio_encodeHeartbeatPayload(uint8_t flags,uint8_t build,uint8_t out[8]){if(!out)return false;clear8(out);if((flags&0xF0U)!=0)return false;out[0]=flags;out[1]=build;return true;}
bool radio_encodeAckPayload(uint8_t result,uint8_t type,uint16_t id,uint8_t stop,uint8_t interlocks,uint8_t out[8]){if(!out)return false;clear8(out);if(result>RADIO_ACK_INTERNAL_FAILURE||type<1||type>10||id==0||stop>2||(interlocks&0xF0U)!=0)return false;out[0]=0;out[1]=result;out[2]=type;put16(out+3,id);out[5]=stop;out[6]=interlocks;return true;}
bool radio_encodeTelemetryPayload(const RadioDecodedPayload &in,uint8_t out[8]){if(!out)return false;clear8(out);switch(in.kind){
case RADIO_PAYLOAD_STATUS:if(in.boatMode>4||in.stopState>2||(in.safetyFlags&0x80U)!=0||(in.batteryPercent>100&&in.batteryPercent!=255))return false;out[0]=1;out[1]=in.boatMode;out[2]=in.stopState;out[3]=in.safetyFlags;out[4]=in.batteryPercent;out[5]=in.gpsFixType;out[6]=in.satellites;return true;
case RADIO_PAYLOAD_BATTERY_SPEED:if(in.speedCmPerSecond!=0xFFFFU&&in.speedCmPerSecond>10000U)return false;out[0]=2;put16(out+1,in.batteryMillivolts);put16(out+3,in.speedCmPerSecond);return true;
case RADIO_PAYLOAD_GNSS_LATITUDE:if(in.coordinateE7 < -900000000L||in.coordinateE7>900000000L)return false;out[0]=3;out[1]=in.coordinateSample;put32(out+2,in.coordinateE7);put16(out+6,in.horizontalAccuracyCm);return true;
case RADIO_PAYLOAD_GNSS_LONGITUDE:if(in.coordinateE7 < -1800000000L||in.coordinateE7>1800000000L)return false;out[0]=4;out[1]=in.coordinateSample;put32(out+2,in.coordinateE7);return true;
case RADIO_PAYLOAD_NAVIGATION:if((in.headingCentidegrees!=0xFFFFU&&in.headingCentidegrees>35999U)||(in.speedCmPerSecond!=0xFFFFU&&in.speedCmPerSecond>10000U)||in.navigationStatus>5)return false;out[0]=5;put16(out+1,in.headingCentidegrees);put16(out+3,in.speedCmPerSecond);out[5]=in.navigationStatus;out[6]=in.waypointIndex;out[7]=in.navigationSample;return true;
case RADIO_PAYLOAD_SONAR_SUMMARY:if(in.sonarHealth>3||(in.sonarAlarmFlags&0xF0U)!=0)return false;out[0]=6;put16(out+1,in.depthCm);put16(out+3,in.sonarRangeCm);out[5]=in.sonarHealth;out[6]=in.sonarAlarmFlags;out[7]=in.sonarSample;return true;
case RADIO_PAYLOAD_WATER_TEMPERATURE:if(in.temperatureCentiC < -500||in.temperatureCentiC>5000)return false;out[0]=10;put16(out+1,(uint16_t)in.temperatureCentiC);out[3]=in.waterTemperatureSample;return true;
case RADIO_PAYLOAD_SAFETY_DIAGNOSTIC: {
        // Only telemetry bits; never encode STOP, ACK or a command.
        radio_r30f::Diagnostic d{};
        d.reason=in.diagnosticReason;
        d.flags=in.diagnosticFlags;
        d.millivolts=in.diagnosticMillivolts;
        d.sequence=in.diagnosticSequence;
        return radio_r30f::encode(d,out);
    }
default:return false;}}
