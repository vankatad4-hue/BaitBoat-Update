#include "boat_telemetry.h"
BoatTelemetry telemetry;
static constexpr uint32_t STATUS_MAX_AGE=1500,BATTERY_MAX_AGE=5000,COORD_PAIR_MAX_GAP=500,GPS_MAX_AGE=2000,NAV_MAX_AGE=2000,SONAR_MAX_AGE=1500;
static bool fresh(uint32_t now,uint32_t stamp,uint32_t maxAge){return stamp!=0&&(uint32_t)(now-stamp)<=maxAge;}
void telemetryInit(){telemetry={};telemetry.batteryPercent=-1;telemetry.stopState=2;}
void telemetryUpdate(const BoatTelemetry &d){telemetry=d;telemetry.receivedAtMs=millis();}
bool telemetryApplyRadioPayload(const RadioDecodedPayload &p,bool primary,uint32_t now){
 switch(p.kind){
 case RADIO_PAYLOAD_STATUS:
  telemetry.statusValid=true;telemetry.statusPrimary=primary;telemetry.reportedMode=p.boatMode;telemetry.stopState=p.stopState;telemetry.safetyFlags=p.safetyFlags;
  telemetry.batteryPercent=(p.batteryPercent==255)?-1:(int)p.batteryPercent;telemetry.gpsFixType=p.gpsFixType;telemetry.gpsSatellites=p.satellites;
  telemetry.gpsConnected=(p.safetyFlags&(1U<<3))!=0;telemetry.autopilotActive=p.boatMode==1||p.boatMode==2;
  telemetry.motorsActive=(p.safetyFlags&(1U<<1))!=0||(p.safetyFlags&(1U<<2))!=0;
  telemetry.statusReceivedAtMs=now;telemetry.receivedAtMs=now;telemetry.radioConnected=true;
  telemetry.batteryPercentValid=(p.batteryPercent!=255);telemetry.batteryPercentReceivedAtMs=now;return true;
 case RADIO_PAYLOAD_BATTERY_SPEED:
  telemetry.batteryVoltageValid=(p.batteryMillivolts!=0xFFFFU);telemetry.batteryMillivolts=p.batteryMillivolts;
  telemetry.batteryVoltage=(p.batteryMillivolts==0xFFFFU)?0.0f:(float)p.batteryMillivolts/1000.0f;
  telemetry.speed=(p.speedCmPerSecond==0xFFFFU)?0.0f:(float)p.speedCmPerSecond/100.0f;telemetry.batteryReceivedAtMs=now;telemetry.receivedAtMs=now;return true;
 case RADIO_PAYLOAD_GNSS_LATITUDE:
  telemetry.latitudeRecordValid=true;telemetry.latitudeSample=p.coordinateSample;telemetry.latitude=(float)p.coordinateE7/10000000.0f;
  telemetry.gpsAccuracyMeters=(p.horizontalAccuracyCm==0xFFFFU)?0.0f:(float)p.horizontalAccuracyCm/100.0f;telemetry.latitudeReceivedAtMs=now;
  telemetry.coordinatePairValid=telemetry.longitudeRecordValid&&telemetry.latitudeSample==telemetry.longitudeSample&&(uint32_t)(now-telemetry.longitudeReceivedAtMs)<=COORD_PAIR_MAX_GAP;
  if(telemetry.coordinatePairValid)telemetry.coordinatePairReceivedAtMs=now;telemetry.receivedAtMs=now;return true;
 case RADIO_PAYLOAD_GNSS_LONGITUDE:
  telemetry.longitudeRecordValid=true;telemetry.longitudeSample=p.coordinateSample;telemetry.longitude=(float)p.coordinateE7/10000000.0f;telemetry.longitudeReceivedAtMs=now;
  telemetry.coordinatePairValid=telemetry.latitudeRecordValid&&telemetry.latitudeSample==telemetry.longitudeSample&&(uint32_t)(now-telemetry.latitudeReceivedAtMs)<=COORD_PAIR_MAX_GAP;
  if(telemetry.coordinatePairValid)telemetry.coordinatePairReceivedAtMs=now;telemetry.receivedAtMs=now;return true;
 case RADIO_PAYLOAD_NAVIGATION:
  telemetry.navigationValid=true;telemetry.heading=(p.headingCentidegrees==0xFFFFU)?-1.0f:(float)p.headingCentidegrees/100.0f;
  telemetry.speed=(p.speedCmPerSecond==0xFFFFU)?0.0f:(float)p.speedCmPerSecond/100.0f;telemetry.navigationStatus=p.navigationStatus;
  telemetry.waypointIndex=p.waypointIndex;telemetry.navigationSample=p.navigationSample;telemetry.navigationReceivedAtMs=now;telemetry.receivedAtMs=now;return true;
 case RADIO_PAYLOAD_SONAR_SUMMARY:
  telemetry.sonarSummaryValid=true;telemetry.sonarDepthCm=p.depthCm;telemetry.sonarRangeCm=p.sonarRangeCm;telemetry.sonarHealth=p.sonarHealth;
  telemetry.sonarAlarmFlags=p.sonarAlarmFlags;telemetry.sonarSample=p.sonarSample;telemetry.sonarReceivedAtMs=now;telemetry.receivedAtMs=now;return true;
 default:return false;
 }
}
void telemetryApplyToBoatState(BoatState &s,uint32_t now){
 const bool statusFresh=telemetry.statusValid&&fresh(now,telemetry.statusReceivedAtMs,STATUS_MAX_AGE);
 const bool voltageFresh=telemetry.batteryVoltageValid&&fresh(now,telemetry.batteryReceivedAtMs,BATTERY_MAX_AGE);
 const bool percentFresh=telemetry.batteryPercentValid&&fresh(now,telemetry.batteryPercentReceivedAtMs,STATUS_MAX_AGE);
 const bool gpsFresh=telemetry.coordinatePairValid&&fresh(now,telemetry.coordinatePairReceivedAtMs,GPS_MAX_AGE);
 const bool navFresh=telemetry.navigationValid&&fresh(now,telemetry.navigationReceivedAtMs,NAV_MAX_AGE);
 const bool sonarFresh=telemetry.sonarSummaryValid&&fresh(now,telemetry.sonarReceivedAtMs,SONAR_MAX_AGE);
 s.batteryVoltage=voltageFresh?telemetry.batteryVoltage:0.0f;s.batteryPercent=percentFresh?telemetry.batteryPercent:0.0f;
 s.latitude=telemetry.latitude;s.longitude=telemetry.longitude;s.heading=(navFresh&&telemetry.heading>=0.0f)?telemetry.heading:0.0f;
 s.speed=voltageFresh||navFresh?telemetry.speed:0.0f;s.gpsFixType=telemetry.gpsFixType;s.gpsSatellites=telemetry.gpsSatellites;
 s.gpsAccuracyMeters=telemetry.gpsAccuracyMeters;s.gpsLastUpdateMs=telemetry.coordinatePairReceivedAtMs;
 s.gpsValid=statusFresh&&gpsFresh&&telemetry.gpsConnected&&telemetry.gpsFixType>=3&&telemetry.latitude>=-90.0f&&telemetry.latitude<=90.0f&&telemetry.longitude>=-180.0f&&telemetry.longitude<=180.0f;
 s.sonarActive=sonarFresh&&(telemetry.sonarHealth==1||telemetry.sonarHealth==2);
 s.motorLeftActive=statusFresh&&(telemetry.safetyFlags&(1U<<1))!=0;s.motorRightActive=statusFresh&&(telemetry.safetyFlags&(1U<<2))!=0;
 if(statusFresh&&telemetry.reportedMode<=2)s.mode=(BoatMode)telemetry.reportedMode;
 if(statusFresh&&telemetry.stopState<=1&&telemetry.statusPrimary){s.emergencyStop=telemetry.stopState==1;s.emergencyStopConfirmed=true;}else s.emergencyStopConfirmed=false;
 s.actuatorSafetyReady=statusFresh&&telemetry.statusPrimary&&telemetry.stopState==0&&(telemetry.safetyFlags&1U)!=0&&(telemetry.safetyFlags&(1U<<6))==0&&s.radioPrimaryConnected&&s.radioActiveLink==RADIO_UI_NRF_PRIMARY;
}
