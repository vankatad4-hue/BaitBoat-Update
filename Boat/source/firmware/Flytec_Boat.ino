/*
 * Flytec Boat non-actuating target application.
 * GPIO assignments are deliberately disabled until the precise Tracker revision,
 * board wiring and external hardware interlocks are verified.
 * No motor/solenoid pins are configured. Do not connect actuator power.
 */
#include <Arduino.h>
#include "boat_dual_radio_port.h"
#include "boat_application_port.h"
#include "tracker_sensor_port.h"
#if __has_include("tracker_commissioned_config.h")
#include "tracker_commissioned_config.h"
static HardwareSerial gnssUart(2);
static TrackerSensorPort sensors(gnssUart,Wire);
static bool sensorsReady=false;
static uint32_t lastReport=0;
#endif
static BoatDualRadioPort radio;
static BoatApplicationPort application(radio);
void setup(){
 Serial.begin(115200);
 Serial.println("Flytec Boat: diagnostic-only firmware; actuator output disabled.");
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
  const uint32_t now=millis();sensors.poll(now);
  if(now-lastReport>=1000U){lastReport=now;BoatHardwareSample s{};sensors.copySensors(s,now);
   Serial.printf("GNSS valid=%d latE7=%ld lonE7=%ld accuracy=%.2f headingValid=%d heading=%.2f voltageValid=%d mV=%u\\n",(int)s.fix.fresh,(long)s.fix.latE7,(long)s.fix.lonE7,s.fix.accuracyM,(int)s.fix.headingValid,s.fix.headingDeg,(int)s.voltageValid,(unsigned)s.batteryMillivolts);
   SensorVector a{},m{};if(sensors.rawMotion(a,m,now))Serial.printf("RAW accel_g %.4f %.4f %.4f mag_uT %.4f %.4f %.4f\\n",a.x,a.y,a.z,m.x,m.y,m.z);
  }
 }
 #endif
 application.tick(); // no commissioned driver attached: diagnostic only
 delay(1);
}
