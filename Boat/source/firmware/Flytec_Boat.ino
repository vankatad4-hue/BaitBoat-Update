/*
 * Flytec Boat non-actuating target application.
 * GPIO assignments are deliberately disabled until the precise ESP32-S3 board revision,
 * board wiring and external hardware interlocks are verified.
 * No motor/solenoid pins are configured. Do not connect actuator power.
 */
#include <Arduino.h>
#include "boat_dual_radio_port.h"
#include "boat_application_port.h"
#include "tracker_sensor_port.h"
#include "mdd20a_direct_profile.h"
#include "figure8_calibration_nvs.h"
#if __has_include("tracker_commissioned_config.h")
#include "tracker_commissioned_config.h"
static HardwareSerial gnssUart(2);
static TrackerSensorPort sensors(gnssUart,Wire);
static bool sensorsReady=false;
static uint32_t lastReport=0,lastWeatherTx=0;
static Figure8AutoState lastFigure8State=Figure8AutoState::IDLE;
#endif
static BoatDualRadioPort radio;
static BoatApplicationPort application(radio);
// Read-only calibration inspection on the prototype diagnostic firmware.
// This does not apply offsets or start automatic figure-eight motion.
static Figure8CalibrationNvs calibrationNvs;
static bool calibrationStorageReady=false;
void setup(){
 Serial.begin(115200);
 Serial.println("Flytec Boat: external E22 UART backup design; uncommissioned radio and actuator GPIO disabled.");
 calibrationStorageReady=calibrationNvs.begin();
 if(calibrationStorageReady){
  Figure8CalibrationNvs::bind(&calibrationNvs);
  Figure8CalibrationStore store({Figure8CalibrationNvs::readBridge,
                                 Figure8CalibrationNvs::writeBridge});
  Figure8StoredCalibration stored{};
  if(store.load(stored))
   Serial.printf("Figure8 NVS: valid record seq=%lu (not applied; no motor output).\\n",
                 (unsigned long)stored.sequence);
  else Serial.println("Figure8 NVS: no valid record; calibration remains unavailable.");
 }else Serial.println("Figure8 NVS: unavailable; calibration remains unavailable.");
 const auto motorProfile=mdd20aUnassignedProfile();
 if(!mdd20aDirectProfileValid(motorProfile,0))Serial.println("MDD20A direct GPIO profile unassigned; no motor outputs configured.");
 const bool ready=radio.begin();
 Serial.println(ready?"Radio transport configured; awaiting secure session.":"Radio pin configuration unavailable.");
 #if __has_include("tracker_commissioned_config.h")
 sensorsReady=sensors.begin(trackerLocalProfile());
 Serial.println(sensorsReady?"Sensor diagnostics enabled; actuator power must remain disconnected.":"Sensor profile denied.");
 #endif
 // No physical stationary/STOP sensor validation exists yet, so leave
 // the mission preparation gate unset (default deny).
}
void loop(){
 #if __has_include("tracker_commissioned_config.h")
 if(sensorsReady){
  // Bench-only serial diagnostics. No actuator commissioning.
  while(Serial.available()){
   const int command=Serial.read();
   if(command=='c')Serial.println(sensors.startAutomaticFigure8(millis())?"FIGURE8: sensor capture started in bench diagnostics only; autonomous propulsion unavailable until safety commissioning.":"FIGURE8: denied; both compasses must be detected.");
   if(command=='x'){sensors.abortAutomaticFigure8();Serial.println("FIGURE8: aborted; no calibration applied.");}
  }
  const uint32_t now=millis();sensors.poll(now);
  const auto calState=sensors.automaticFigure8State();
  if(calState!=lastFigure8State){
   lastFigure8State=calState;
   if(calState==Figure8AutoState::CANDIDATE_READY){
    Figure8Candidate gy{},m{};
    if(sensors.automaticFigure8Candidates(gy,m))Serial.printf("FIGURE8 AUTO CANDIDATES (NOT APPLIED): GY91 xy=(%.2f,%.2f) scales=(%.3f,%.3f), M100 xy=(%.2f,%.2f) scales=(%.3f,%.3f)\\n",gy.offsetX,gy.offsetY,gy.scaleX,gy.scaleY,m.offsetX,m.offsetY,m.scaleX,m.scaleY);
   }else if(calState==Figure8AutoState::TIMED_OUT)Serial.println("FIGURE8: timeout or sensor loss; rejected.");
  }
  if(now-lastReport>=1000U){lastReport=now;BoatHardwareSample s{};sensors.copySensors(s,now);
   Serial.printf("GNSS valid=%d latE7=%ld lonE7=%ld accuracy=%.2f headingValid=%d heading=%.2f voltageValid=%d mV=%u\\n",(int)s.fix.fresh,(long)s.fix.latE7,(long)s.fix.lonE7,s.fix.accuracyM,(int)s.fix.headingValid,s.fix.headingDeg,(int)s.voltageValid,(unsigned)s.batteryMillivolts);
   int32_t t=0;uint32_t p=0;
   if(sensors.weather(t,p,now))Serial.printf("BMP280 temp=%.2f C pressure=%.2f hPa\\n",t/100.0f,p/100.0f);
   SensorVector a{},m{};if(sensors.rawMotion(a,m,now))Serial.printf("RAW accel_g %.4f %.4f %.4f mag_uT %.4f %.4f %.4f\\n",a.x,a.y,a.z,m.x,m.y,m.z);
  }
  if(now-lastWeatherTx>=5000U){
   lastWeatherTx=now;int32_t t=0;uint32_t p=0;
   if(sensors.weather(t,p,now)){
    RadioDecodedPayload weather{};weather.kind=RADIO_PAYLOAD_WEATHER;
    weather.temperatureCentiC=(int16_t)t;weather.pressurePa=p;
    radio.sendHardwareTelemetry(weather);
   }
  }
 }
 #endif
 application.tick(); // no commissioned driver attached: diagnostic only
 delay(1);
}
