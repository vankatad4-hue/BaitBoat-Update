#include "boat_telemetry.h"
#include "sonar_echo_profile_v1.h"
#include <string.h>
#include <assert.h>
uint32_t millis(){return 0;}
static RadioDecodedPayload p(RadioPayloadKind k){RadioDecodedPayload x{};x.kind=k;return x;}
int main(){
 telemetryInit();BoatState s{};s.radioPrimaryConnected=true;s.radioActiveLink=RADIO_UI_NRF_PRIMARY;
 auto st=p(RADIO_PAYLOAD_STATUS);st.boatMode=1;st.stopState=0;st.safetyFlags=0x19;st.batteryPercent=82;st.gpsFixType=3;st.satellites=9;assert(telemetryApplyRadioPayload(st,true,1000));
 auto lat=p(RADIO_PAYLOAD_GNSS_LATITUDE);lat.coordinateSample=22;lat.coordinateE7=420000000;lat.horizontalAccuracyCm=250;assert(telemetryApplyRadioPayload(lat,true,1100));
 auto lon=p(RADIO_PAYLOAD_GNSS_LONGITUDE);lon.coordinateSample=22;lon.coordinateE7=250000000;assert(telemetryApplyRadioPayload(lon,true,1150));
 auto nav=p(RADIO_PAYLOAD_NAVIGATION);nav.headingCentidegrees=9000;nav.speedCmPerSecond=120;nav.navigationStatus=1;nav.waypointIndex=2;assert(telemetryApplyRadioPayload(nav,true,1200));
 auto bat=p(RADIO_PAYLOAD_BATTERY_SPEED);bat.batteryMillivolts=12000;bat.speedCmPerSecond=120;assert(telemetryApplyRadioPayload(bat,true,1250));
 auto sonar=p(RADIO_PAYLOAD_SONAR_SUMMARY);sonar.sonarHealth=1;sonar.depthCm=540;assert(telemetryApplyRadioPayload(sonar,true,1300));
 telemetryApplyToBoatState(s,1400);assert(s.mode==MODE_AUTOPILOT&&s.emergencyStopConfirmed&&!s.emergencyStop&&s.actuatorSafetyReady);
 assert(s.gpsValid&&s.latitude==42.0f&&s.longitude==25.0f&&s.heading==90.0f&&s.speed==1.2f);
 assert(s.batteryVoltage==12.0f&&s.batteryPercent==82&&s.sonarActive&&s.headingValid&&!s.lowBattery&&!s.failsafeActive);
 st.safetyFlags=0x39;st.batteryPercent=18;
 assert(telemetryApplyRadioPayload(st,true,1450));telemetryApplyToBoatState(s,1460);assert(s.lowBattery&&!s.criticalBattery&&s.batteryPercent==18);
 st.safetyFlags=0xB9;
 assert(telemetryApplyRadioPayload(st,true,1465));telemetryApplyToBoatState(s,1466);assert(s.lowBattery&&s.criticalBattery);
 st.safetyFlags=0x19;st.batteryPercent=42;
 assert(telemetryApplyRadioPayload(st,true,1470));telemetryApplyToBoatState(s,1480);assert(!s.lowBattery&&s.batteryPercent==42);
 // Loss of current primary RF must immediately revoke cached safety-ready UI.
 s.radioPrimaryConnected=false;s.radioActiveLink=RADIO_UI_NONE;
 telemetryApplyToBoatState(s,1490);assert(!s.actuatorSafetyReady);
 s.radioPrimaryConnected=true;s.radioActiveLink=RADIO_UI_NRF_PRIMARY;
 assert(telemetryApplyRadioPayload(st,false,1500));telemetryApplyToBoatState(s,1600);assert(!s.emergencyStopConfirmed&&!s.actuatorSafetyReady);
 lat.coordinateSample=23;assert(telemetryApplyRadioPayload(lat,true,1700));telemetryApplyToBoatState(s,1800);assert(!s.gpsValid);
 telemetryApplyToBoatState(s,3201);assert(!s.emergencyStopConfirmed&&!s.actuatorSafetyReady&&!s.gpsValid&&!s.headingValid);
 telemetryInit();
 uint8_t profile[sonar_profile_v1::BIN_COUNT];
 for(uint16_t i=0;i<sonar_profile_v1::BIN_COUNT;++i)profile[i]=static_cast<uint8_t>(255U-i);
 // Out-of-order fragments assemble only into the complete 128-bin profile.
 for(int chunk=sonar_profile_v1::CHUNK_COUNT-1;chunk>=0;--chunk){
  uint8_t wire[8]{};assert(sonar_profile_v1::encode(23U,static_cast<uint8_t>(chunk),profile,wire));
  RadioDecodedPayload fragment{};assert(radio_decodeTelemetryPayload(wire,fragment));
  if(chunk==sonar_profile_v1::CHUNK_COUNT-1)assert(!telemetryApplyRadioPayload(fragment,false,4000));
  assert(telemetryApplyRadioPayload(fragment,true,4000U+static_cast<uint32_t>(sonar_profile_v1::CHUNK_COUNT-chunk)));
 }
 telemetryApplyToBoatState(s,4100);
 assert(s.sonarEchoProfileValid&&s.sonarEchoProfileId==23);
 assert(memcmp(s.sonarEchoProfile,profile,sizeof(profile))==0);
 return 0;
}
