#pragma once
#include "boat_sd_module_arduino.h"
#include "figure8_calibration_store.h"
// One backend instance per Boat. Slots are separate fixed-size files;
// corrupt/torn writes cannot destroy the other CRC-valid slot.
class BoatSdCalibrationBackend {
public:
 explicit BoatSdCalibrationBackend(BoatSdModuleArduino &sd):sd_(sd){}
 bool readSlot(uint8_t slot,Figure8StoredCalibration &r){
  if(slot>1)return false;
  return sd_.read(slot==0?"/cal_f8_a.bin":"/cal_f8_b.bin",
                  reinterpret_cast<uint8_t*>(&r),sizeof(r));
 }
 bool writeSlot(uint8_t slot,const Figure8StoredCalibration &r){
  if(slot>1||!figure8CalibrationValid(r))return false;
  return sd_.write(slot==0?"/cal_f8_a.bin":"/cal_f8_b.bin",
                   reinterpret_cast<const uint8_t*>(&r),sizeof(r));
 }
 // Bridge callbacks for Figure8StoreIo, assigned only in single-instance setup.
 static void bind(BoatSdCalibrationBackend *p){instance_=p;}
 static bool readBridge(uint8_t slot,Figure8StoredCalibration &r){
  return instance_&&instance_->readSlot(slot,r);
 }
 static bool writeBridge(uint8_t slot,const Figure8StoredCalibration &r){
  return instance_&&instance_->writeSlot(slot,r);
 }
private:
 BoatSdModuleArduino &sd_;
 static BoatSdCalibrationBackend *instance_;
};
