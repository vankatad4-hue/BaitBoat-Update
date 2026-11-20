#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include "tracker_gnss.h"
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
private:
 HardwareSerial &serial_;TwoWire &wire_;
 TrackerSensorConfig cfg_{};
 TrackerGnss gnss_;
 bool configured_=false,imuReady_=false,headingValid_=false,voltageValid_=false,adcPending_=false;
 double heading_=0;uint16_t voltage_=0;
 uint32_t headingAt_=0,voltageAt_=0,adcAt_=0,imuPollAt_=0;
 double adjust_[3]={1,1,1};
 bool writeReg(uint8_t address,uint8_t reg,uint8_t value);
 bool readRegs(uint8_t address,uint8_t reg,uint8_t *dst,uint8_t length);
 bool writeAdc(uint16_t value);
 bool initializeImu();
 void pollImu(uint32_t now);
 void pollAdc(uint32_t now);
};
