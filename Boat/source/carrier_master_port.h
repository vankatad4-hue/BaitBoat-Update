#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "carrier_link.h"
// Explicitly initialized with the audited shared I2C bus.
// No actuator attachment, no automatic arm, no queued motion.
class CarrierMasterPort{
public:
 explicit CarrierMasterPort(TwoWire &wire):wire_(wire){}
 bool syncOff();
 bool disarm();
 bool command(int16_t left,int16_t right,int8_t coil);
 bool readStatus(carrier::Status &status);
 bool healthy(uint32_t now)const{return haveStatus_&&now-at_<30U;}
private:
 TwoWire &wire_;
 uint32_t sequence_=0,at_=0;
 uint16_t lastSample_=0;
 bool haveStatus_=false;
 bool send(carrier::Op op,int16_t left,int16_t right,int8_t coil);
};
