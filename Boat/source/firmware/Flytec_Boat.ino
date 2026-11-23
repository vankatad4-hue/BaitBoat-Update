/*
 * Flytec Boat non-actuating target application.
 * GPIO assignments are deliberately disabled until the precise ESP32-S3 board revision,
 * board wiring and external hardware interlocks are verified.
 * No motor/solenoid pins are configured. Do not connect actuator power.
 */
#include <Arduino.h>
#include <sys/time.h>
#include "radio_utc_time_v1.h"
#include "boat_dual_radio_port.h"
#include "boat_application_port.h"
#include "tracker_sensor_port.h"
#include "mdd20a_direct_profile.h"
#include "figure8_calibration_nvs.h"
#include "boat_home_point_nvs.h"
#include "boat_endpoint_memory_gate.h"
#include "boat_common_bus_readonly_port_r30d.h"
#include "r30h10_gpio_test_profile.h" // passive constants; no hardware outputs
#include "boat_r30e_fault_status.h"

// R30D: compile the prospective input-only common-bus feedback contract on
// the real ESP32-S3 target without constructing a reader or touching GPIO.
// These constants must stay locked until independent electrical acceptance.
static_assert(!boat_r30d::ACTUAL_READER_COMMISSIONED &&
              !boat_r30c::COMMON_BUS_HARDWARE_COMMISSIONED,
              "R30D diagnostic build must not commission actuator power");
#if __has_include("tracker_commissioned_config.h")
#include "tracker_commissioned_config.h"
static HardwareSerial gnssUart(2);
static TrackerSensorPort sensors(gnssUart,Wire);
static flytec_time::Discipline gnssUtcClock;
static uint32_t lastGpsUtcStamp=0;
static bool haveGpsUtcStamp=false;
static bool sensorsReady=false;
static uint32_t lastReport=0,lastWeatherTx=0;
static Figure8AutoState lastFigure8State=Figure8AutoState::IDLE;
#endif
// R30E local diagnostic telemetry ONLY. There is no selected physical
// safety-input reader, so this always reports NO_READER/UNVERIFIED, never an
// invented good contact or an authenticated radio telemetry payload.
static boat_r30d::ReadOnlyCommonBusPort safetyObservations;
static boat_r30e::ChangeTracker safetyFaultTransitions;
static uint32_t lastSafetyReportAt=0;
static uint8_t safetyDiagnosticSequence=0;
static BoatDualRadioPort radio;
static BoatApplicationPort application(radio);
static BoatHomePointNvs homePointNvs;
static bool homeStorageReady=false;
// Read-only calibration inspection on the prototype diagnostic firmware.
// This does not apply offsets or start automatic figure-eight motion.
static Figure8CalibrationNvs calibrationNvs;
static bool calibrationStorageReady=false;
void setup(){
 // R30H-63: runtime affinity evidence; no new task and no shared-state race.
 Serial.printf("R30H-63 Boat setup core=%d CPUs=%d\\n", xPortGetCoreID(), portNUM_PROCESSORS);
 Serial.begin(115200);
 Serial.println("Flytec Boat: external E22 UART backup design; uncommissioned radio and actuator GPIO disabled.");
 const uint32_t flashBytes=(uint32_t)ESP.getFlashChipSize();
 const uint32_t psramBytes=(uint32_t)ESP.getPsramSize();
 Serial.printf("Boat endpoint: flash=%lu PSRAM=%lu, selected N32R16V: %s\\n",
     (unsigned long)flashBytes,(unsigned long)psramBytes,
     boatEndpointMemoryMatches(flashBytes,psramBytes)?"MEMORY MATCH ONLY":"MISMATCH - DIAGNOSTIC ONLY");

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
 homeStorageReady=homePointNvs.begin();
 if(homeStorageReady){
  const BoatHomePointStore store={BoatHomePointNvs::loadBridge,BoatHomePointNvs::saveBridge};
  radio.setHomePointStore(store);
  BoatHomePoint storedHome{};
  if(homePointNvs.load(storedHome))
   Serial.printf("Boat Home NVS: valid latE7=%ld lonE7=%ld seq=%lu\\n",(long)storedHome.latE7,(long)storedHome.lonE7,(unsigned long)storedHome.sequence);
  else Serial.println("Boat Home NVS: empty; Home will be saved and read back during mission preparation.");
 }else Serial.println("Boat Home NVS: unavailable; mission preparation and automatic RTH remain disabled.");
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
 // Print only once to confirm the actual Arduino loop core on target hardware.
 static bool r30h63CoreReported=false;
 if(!r30h63CoreReported){r30h63CoreReported=true;
  Serial.printf("R30H-63 Boat loop core=%d\\n",xPortGetCoreID());
 }
 #if __has_include("tracker_commissioned_config.h")
 if(sensorsReady){
  // Bench-only serial diagnostics. No actuator commissioning.
  while(Serial.available()){
   const int command=Serial.read();
   if(command=='c')Serial.println(sensors.startAutomaticFigure8(millis())?"FIGURE8: sensor capture started in bench diagnostics only; autonomous propulsion unavailable until safety commissioning.":"FIGURE8: denied; both compasses must be detected.");
   if(command=='x'){sensors.abortAutomaticFigure8();Serial.println("FIGURE8: aborted; no calibration applied.");}
  }
  const uint32_t now=millis();sensors.poll(now);
  const auto utc=sensors.utcSnapshot(now);
  // The sensor configuration is physically audited before begin().
  // The received GNSS serial-time sample has no PPS; UTC is COARSE,
  // NEVER a timestamp for physical RF/STOP end-to-end certification.
  radio.offerGnssUtc(utc);
  if(flytec_utc::fresh(utc,now)&&
     (!haveGpsUtcStamp||utc.localMs!=lastGpsUtcStamp)){
    haveGpsUtcStamp=true;lastGpsUtcStamp=utc.localMs;
    uint64_t currentUtc=0;
    if(flytec_utc::extrapolate(utc,now,currentUtc)&&
       gnssUtcClock.observe(currentUtc,now)&&
       gnssUtcClock.shouldSetRtc(now)&&
       gnssUtcClock.utcNow(now,currentUtc)){
      timeval tv{};tv.tv_sec=(time_t)(currentUtc/1000ULL);
      tv.tv_usec=(suseconds_t)((currentUtc%1000ULL)*1000ULL);
      // Only CLOCK_REALTIME is updated. millis()/radio lease/RTH remains
      // monotonic and is deliberately NEVER synchronized or stepped.
      if(settimeofday(&tv,nullptr)==0)gnssUtcClock.markRtcSet(now);
    }
  }
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
 // R30E: emit a diagnostic-only record through USB serial; no safety
 // hardware is installed or read, and NO RF/actuator output uses this data.
 // A future inspected reader would populate safetyObservations without
 // changing the rule that an observation cannot commission its own source.
 const uint32_t diagnosticNow=millis();
 if((uint32_t)(diagnosticNow-lastSafetyReportAt)>=2000U){
  lastSafetyReportAt=diagnosticNow;
  const auto cap=safetyObservations.snapshot(diagnosticNow);
  const auto status=boat_r30e::makeStatus(
      cap,diagnosticNow,++safetyDiagnosticSequence);
  const bool changed=safetyFaultTransitions.observe(status);
  uint8_t localBytes[8]{};
  boat_r30e::encodeLocal(status,localBytes);
  // R30F: a DIFFERENT, strictly versioned TELEMETRY subtype 8 may be sent
  // over the existing authenticated primary nRF session. It contains ONLY
  // unverified diagnostic flags and never creates a STOP ACK or command.
  // The radio port itself refuses transmission without a provisioned key,
  // authenticated session and fresh primary link. Do not send via backup.
  RadioDecodedPayload safetyTelemetry{};
  safetyTelemetry.kind=RADIO_PAYLOAD_SAFETY_DIAGNOSTIC;
  safetyTelemetry.diagnosticReason=static_cast<uint8_t>(status.reason);
  safetyTelemetry.diagnosticFlags=status.flags;
  safetyTelemetry.diagnosticMillivolts=status.millivolts;
  safetyTelemetry.diagnosticSequence=status.sequence;
  radio.sendHardwareTelemetry(safetyTelemetry);
  // The D5 frame below remains USB-local, NOT the authenticated RF format.
  // It is never interpreted as a command or verified physical contact.
  Serial.printf("R30E,LOCAL_ONLY,%s,flags=0x%04X,pack_mV=%u,seq=%u,changed=%u,outputs=LOCKED,frame=%02X%02X%02X%02X%02X%02X%02X%02X\n",
      boat_r30e::label(status.reason),
      (unsigned)status.flags,(unsigned)status.millivolts,
      (unsigned)status.sequence,(unsigned)changed,
      (unsigned)localBytes[0],(unsigned)localBytes[1],
      (unsigned)localBytes[2],(unsigned)localBytes[3],
      (unsigned)localBytes[4],(unsigned)localBytes[5],
      (unsigned)localBytes[6],(unsigned)localBytes[7]);
 }
 application.tick(); // no commissioned driver attached: diagnostic only
#if BOAT_R30H_LOCAL_TRACE_ENABLED
 // R30H-2 ONLY: drain at most one bounded record per loop after RF/safety
 // servicing. No Serial I/O occurs in authenticated receive/watchdog paths.
 // These are Boat-local millis() and LOGICAL INTENTS, not current-zero proof.
 if(Serial && Serial.availableForWrite()>=120){
   boat_r30h2::Record ev{};
   if(radio.takeR30HTrace(ev)){
     Serial.printf("R30H2,BOAT_LOCAL,%lu,%lu,%s,%u,%u,%u,%lu,INTENT_ONLY\n",
       (unsigned long)ev.timestampMs,(unsigned long)ev.lastAcceptedRxMs,
       boat_r30h2::label(ev.event),(unsigned)ev.link,
       (unsigned)ev.freshMask,(unsigned)ev.commandId,
       (unsigned long)radio.r30hTraceOverflows());
   }
 }
#endif
 delay(1);
}
