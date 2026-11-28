#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include "tracker_gnss.h"
#include "m100_ubx_nav_pvt.h"
#include "bmp280_compensation.h"
#include "compass_calibration_session.h"
#include "figure8_compass_calibration.h"
#include "figure8_auto_session.h"
#include "m100_compass_qmc5883l.h"
#include "carrier_sensor_math.h"
#include "boat_hardware_supervisor.h"
#include "ina231_acquisition.h"
#include "open_echo_dbt.h"
// Logic-only sensor driver. Caller must establish verified GNSS power/reset
// and choose exact UART/I2C header pins. Does not set safety or stationary flags.
struct TrackerSensorConfig{
 bool pinAuditSigned;
 int8_t gnssRx,gnssTx,sda,scl;
 uint32_t gnssBaud;
 uint8_t imuAddress,adcAddress;
 CarrierHeadingCalibration heading;
 CarrierBatteryCalibration battery;
 // R28 INA231 selected; all three local signoff gates default false.
 bool ina231BoardAndWiringSigned=false;
 bool ina231DmmCalibrationSigned=false;
 uint8_t ina231Address=boat_r27::I2C_CANDIDATE_ADDRESS;
};
class TrackerSensorPort{
public:
 TrackerSensorPort(HardwareSerial &serial,TwoWire &wire):serial_(serial),wire_(wire){}
 bool begin(const TrackerSensorConfig &cfg);
 void poll(uint32_t now);
 void copySensors(BoatHardwareSample &sample,uint32_t now)const;
 // Normalized acquisition-to-telemetry boundary. A future direct TUSS4470
 // reader or the DBT adapter may publish here after validating a real depth;
 // this method itself performs no bus I/O and assigns no pins.
 bool publishSonarSummary(uint16_t depthCm,uint16_t rangeCm,uint8_t health,uint8_t alarmFlags,uint8_t sample,uint32_t now);
 // Publish an already measured 128-bin envelope from the sonar acquisition
 // task. This boundary performs no sampling or ADC scaling itself.
 bool publishSonarEchoProfile(uint8_t profileId,const uint8_t bins[128],uint32_t now);
 // Feed bytes from a dedicated, verified Open Echo NMEA-DBT UART adapter.
 // This parser call performs no UART reads and does not assign board pins.
 bool feedOpenEchoDbtByte(char byte,uint32_t now);
 // Bounded adapter for an owner-supplied dedicated serial stream. The caller
 // chooses and verifies that stream/pin mapping; this method never configures it.
 void pollOpenEchoDbt(Stream &stream,uint32_t now);
 void invalidateSonar(){sonarValid_=false;}
 // Accepts already converted Celsius from the eventual waterproof probe.
 // No water-temperature sensor model or electrical interface is selected yet.
 bool publishWaterTemperature(int16_t temperatureCentiC,uint8_t sample,uint32_t now);
 void invalidateWaterTemperature(){waterTemperatureValid_=false;}
 // Fresh receiver UTC, separate from position/actuator safety. No PPS.
 flytec_utc::Sample utcSnapshot(uint32_t now)const{
  if(!configured_)return {};
  return cfg_.gnssBaud==115200?
      m100Ubx_.snapshot(now).utc:gnss_.utcSnapshot(now);
 }
 bool imuReady()const{return imuReady_;}
 bool m100CompassReady()const{return m100CompassReady_;}
 bool startAutomaticFigure8(uint32_t now){return autoFigure8_.start(now,imuReady_,m100CompassReady_);}
 void abortAutomaticFigure8(){autoFigure8_.abort();}
 Figure8AutoState automaticFigure8State()const{return autoFigure8_.state();}
 bool automaticFigure8Candidates(Figure8Candidate &gy,Figure8Candidate &m100)const{return autoFigure8_.candidates(gy,m100);}
 bool startFigure8Calibration(){if(!imuReady_||!m100CompassReady_)return false;gy91Figure8_.start();m100Figure8_.start();return true;}
 bool finishFigure8Calibration(Figure8Candidate &gy91,Figure8Candidate &m100){
  const bool a=gy91Figure8_.finish(gy91),b=m100Figure8_.finish(m100);return a&&b;
 }
 bool beginCompassCalibration(){if(!imuReady_)return false;compassCal_.start();return true;}
 bool finishCompassCalibration(CompassCalibrationCandidate &candidate){return compassCal_.finish(candidate);}
 bool compassCalibrationActive()const{return compassCal_.active();}
 bool weather(int32_t &temperatureCentiC,uint32_t &pressurePa,uint32_t now)const{
  if(!bmpValid_||(uint32_t)(now-bmpAt_)>5000U)return false;
  temperatureCentiC=bmpTemp_;pressurePa=bmpPressure_;return true;
 }
 bool rawMotion(SensorVector &a,SensorVector &m,uint32_t now)const{if(!rawValid_||now-rawAt_>=300U)return false;a=rawAccel_;m=rawMag_;return true;}
private:
 HardwareSerial &serial_;TwoWire &wire_;
 TrackerSensorConfig cfg_{};
 TrackerGnss gnss_;
 M100UbxNavPvt m100Ubx_;
 CompassCalibrationSession compassCal_;
 Figure8AutoSession autoFigure8_;
 Figure8CompassCalibration gy91Figure8_,m100Figure8_;
 bool m100CompassReady_=false;uint32_t m100CompassPollAt_=0;
 bool bmpReady_=false,bmpValid_=false;uint8_t bmpAddress_=0;
 Bmp280Calibration bmpCal_{};int32_t bmpTemp_=0;uint32_t bmpPressure_=0,bmpAt_=0,bmpPollAt_=0;
 bool sonarValid_=false;uint16_t sonarDepthCm_=0,sonarRangeCm_=0;uint8_t sonarHealth_=0,sonarAlarmFlags_=0,sonarSample_=0;uint32_t sonarAt_=0;
 bool sonarProfileValid_=false;uint8_t sonarProfileId_=0,sonarProfileBins_[128]{};uint32_t sonarProfileAt_=0;
 OpenEchoDbtDecoder openEchoDbt_{};uint8_t openEchoSample_=0;
 bool waterTemperatureValid_=false;int16_t waterTemperatureCentiC_=0;uint8_t waterTemperatureSample_=0;uint32_t waterTemperatureAt_=0;
 bool configured_=false,imuReady_=false,headingValid_=false,voltageValid_=false,adcPending_=false;
 bool ina231Ready_=false;
 boat_r28::VoltageSampler ina231Sampler_{};
 double heading_=0;uint16_t voltage_=0;
 uint32_t headingAt_=0,voltageAt_=0,adcAt_=0,imuPollAt_=0;
 double adjust_[3]={1,1,1};
 SensorVector rawAccel_{},rawMag_{};uint32_t rawAt_=0;bool rawValid_=false;
 bool writeReg(uint8_t address,uint8_t reg,uint8_t value);
 bool readRegs(uint8_t address,uint8_t reg,uint8_t *dst,uint8_t length);
 bool writeAdc(uint16_t value);
 bool initializeImu();
 bool initializeBmp();void pollBmp(uint32_t now);
 bool initializeM100Compass();void pollM100Compass(uint32_t now);
 void pollImu(uint32_t now);
 void pollAdc(uint32_t now);
};
