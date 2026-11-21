#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include "tracker_gnss.h"
#include "bmp280_compensation.h"
#include "carrier_sensor_math.h"
#include "boat_hardware_supervisor.h"
// Logic-only sensor driver. Caller must establish verified GNSS power/reset
// and choose exact UART/I2C header pins. Does not set safety or stationary flags.
struct TrackerSensorConfig{
 bool pinAuditSigned;
 int8_t gnssRx,gnssTx,sda,scl;
 uint32_t gnssBaud;
 uint8_t imuAddress,adcAddress;
 CarrierHeadingCalibration heading;
 CarrierBatteryCalibration battery;
};
class TrackerSensorPort{
public:
 TrackerSensorPort(HardwareSerial &serial,TwoWire &wire):serial_(serial),wire_(wire){}
 bool begin(const TrackerSensorConfig &cfg);
 void poll(uint32_t now);
 void copySensors(BoatHardwareSample &sample,uint32_t now)const;
 bool imuReady()const{return imuReady_;}
 bool weather(int32_t &temperatureCentiC,uint32_t &pressurePa,uint32_t now)const{
  if(!bmpValid_||(uint32_t)(now-bmpAt_)>5000U)return false;
  temperatureCentiC=bmpTemp_;pressurePa=bmpPressure_;return true;
 }
 bool rawMotion(SensorVector &a,SensorVector &m,uint32_t now)const{if(!rawValid_||now-rawAt_>=300U)return false;a=rawAccel_;m=rawMag_;return true;}
private:
 HardwareSerial &serial_;TwoWire &wire_;
 TrackerSensorConfig cfg_{};
 TrackerGnss gnss_;
 bool bmpReady_=false,bmpValid_=false;uint8_t bmpAddress_=0;
 Bmp280Calibration bmpCal_{};int32_t bmpTemp_=0;uint32_t bmpPressure_=0,bmpAt_=0,bmpPollAt_=0;
 bool configured_=false,imuReady_=false,headingValid_=false,voltageValid_=false,adcPending_=false;
 double heading_=0;uint16_t voltage_=0;
 uint32_t headingAt_=0,voltageAt_=0,adcAt_=0,imuPollAt_=0;
 double adjust_[3]={1,1,1};
 SensorVector rawAccel_{},rawMag_{};uint32_t rawAt_=0;bool rawValid_=false;
 bool writeReg(uint8_t address,uint8_t reg,uint8_t value);
 bool readRegs(uint8_t address,uint8_t reg,uint8_t *dst,uint8_t length);
 bool writeAdc(uint16_t value);
 bool initializeImu();
 bool initializeBmp();void pollBmp(uint32_t now);
 void pollImu(uint32_t now);
 void pollAdc(uint32_t now);
};
