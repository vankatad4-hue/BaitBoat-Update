#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "figure8_calibration_store.h"
// Boat-only NVS backend. Remote SD is used for maps and waypoints.
// Each Preferences key is committed independently; inactive-slot writes
// preserve the previous CRC-valid record if power is interrupted.
class Figure8CalibrationNvs {
public:
 bool begin(){return prefs_.begin("f8cal",false);}
 void end(){prefs_.end();}
 bool readSlot(uint8_t slot,Figure8StoredCalibration &out){
  if(slot>1)return false;
  const char *key=slot==0?"slot_a":"slot_b";
  if(prefs_.getBytesLength(key)!=sizeof(out))return false;
  return prefs_.getBytes(key,&out,sizeof(out))==sizeof(out);
 }
 bool writeSlot(uint8_t slot,const Figure8StoredCalibration &value){
  if(slot>1||!figure8CalibrationValid(value))return false;
  const char *key=slot==0?"slot_a":"slot_b";
  return prefs_.putBytes(key,&value,sizeof(value))==sizeof(value);
 }
 // Callback bridge must be bound to the single Boat NVS instance.
 static void bind(Figure8CalibrationNvs *p){instance_=p;}
 static bool readBridge(uint8_t slot,Figure8StoredCalibration &r){
  return instance_&&instance_->readSlot(slot,r);
 }
 static bool writeBridge(uint8_t slot,const Figure8StoredCalibration &r){
  return instance_&&instance_->writeSlot(slot,r);
 }
private:
 Preferences prefs_{};
 static Figure8CalibrationNvs *instance_;
};
