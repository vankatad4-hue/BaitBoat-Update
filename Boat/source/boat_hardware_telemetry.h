#pragma once
#include "boat_hardware_status.h"
// One coherent publish pass can now include compact sensor summaries while
// keeping the RF work bounded to at most eight fixed-size payloads.
struct BoatTelemetryBatch{RadioDecodedPayload payload[8];uint8_t count;};
inline BoatTelemetryBatch boatHardware_telemetry(const BoatHardwareSample &s,uint32_t now,uint8_t id,uint8_t waypoint,MissionControlPhase phase){
 BoatTelemetryBatch b{};auto in=boatHardware_inputs(s,now,false);
 const bool speed=s.speedValid&&now-s.speedAt<2000U&&s.speedCmPerSecond<=10000;
 auto &battery=b.payload[b.count++];battery.kind=RADIO_PAYLOAD_BATTERY_SPEED;
 battery.batteryMillivolts=s.voltageValid&&now-s.batteryAt<1000U&&s.batteryMillivolts>0&&s.batteryMillivolts<65535?s.batteryMillivolts:0xffff;
 battery.speedCmPerSecond=speed?s.speedCmPerSecond:0xffff;
 auto &nav=b.payload[b.count++];nav.kind=RADIO_PAYLOAD_NAVIGATION;
 const bool heading=in.fix.headingValid&&std::isfinite(in.fix.headingDeg)&&in.fix.headingDeg>=0&&in.fix.headingDeg<360;
 nav.headingCentidegrees=heading?(uint16_t)std::fmin(35999.0,std::round(in.fix.headingDeg*100.0)):0xffff;
 nav.speedCmPerSecond=battery.speedCmPerSecond;nav.navigationSample=id;nav.waypointIndex=waypoint;
 nav.navigationStatus=phase==MC_RETURNING?2:phase==MC_OUTBOUND?1:phase==MC_COMPLETE?3:phase==MC_ABORTED?4:0;
 const auto &f=in.fix;
 if(f.fresh&&s.gpsFixType>=3&&std::isfinite(f.accuracyM)&&f.accuracyM>0&&f.accuracyM<=655.34&&f.latE7>=-900000000&&f.latE7<=900000000&&f.lonE7>=-1800000000&&f.lonE7<=1800000000){
  auto &lat=b.payload[b.count++];lat.kind=RADIO_PAYLOAD_GNSS_LATITUDE;lat.coordinateSample=id;lat.coordinateE7=f.latE7;lat.horizontalAccuracyCm=(uint16_t)std::ceil(f.accuracyM*100.0);
  auto &lon=b.payload[b.count++];lon.kind=RADIO_PAYLOAD_GNSS_LONGITUDE;lon.coordinateSample=id;lon.coordinateE7=f.lonE7;
 }
 if(s.sonarValid&&(uint32_t)(now-s.sonarAt)<=1500U&&(s.sonarHealth==1||s.sonarHealth==2)&&s.sonarRangeCm>0&&s.sonarDepthCm<=s.sonarRangeCm&&(s.sonarAlarmFlags&0xF0U)==0){
  auto &sonar=b.payload[b.count++];sonar.kind=RADIO_PAYLOAD_SONAR_SUMMARY;sonar.depthCm=s.sonarDepthCm;sonar.sonarRangeCm=s.sonarRangeCm;
  sonar.sonarHealth=s.sonarHealth;sonar.sonarAlarmFlags=s.sonarAlarmFlags;sonar.sonarSample=s.sonarSample;
 }
 if(s.waterTemperatureValid&&(uint32_t)(now-s.waterTemperatureAt)<=5000U&&s.waterTemperatureCentiC>=-500&&s.waterTemperatureCentiC<=5000){
  auto &water=b.payload[b.count++];water.kind=RADIO_PAYLOAD_WATER_TEMPERATURE;water.temperatureCentiC=s.waterTemperatureCentiC;water.waterTemperatureSample=s.waterTemperatureSample;
 }
 if(s.weatherValid&&(uint32_t)(now-s.weatherAt)<=5000U&&s.weatherTemperatureCentiC>=-4000&&s.weatherTemperatureCentiC<=8500&&s.pressurePa>=30000&&s.pressurePa<=120000){
  auto &weather=b.payload[b.count++];weather.kind=RADIO_PAYLOAD_WEATHER;weather.temperatureCentiC=s.weatherTemperatureCentiC;weather.pressurePa=s.pressurePa;
 }
 return b;
}
