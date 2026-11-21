#pragma once
#include <stdint.h>
#include "figure8_calibration_record.h"
// Storage adapter uses two separately addressed slots; backend must write
// complete records atomically or fail. Never overwrite the current good slot.
struct Figure8StoreIo {
 bool (*readSlot)(uint8_t,Figure8StoredCalibration &)=nullptr;
 bool (*writeSlot)(uint8_t,const Figure8StoredCalibration &)=nullptr;
};
class Figure8CalibrationStore {
public:
 explicit Figure8CalibrationStore(Figure8StoreIo io):io_(io){}
 bool load(Figure8StoredCalibration &out)const{
  if(!io_.readSlot)return false;
  Figure8StoredCalibration a{},b{};
  if(!io_.readSlot(0,a))a={};
  if(!io_.readSlot(1,b))b={};
  return figure8SelectRecord(a,b,out);
 }
 bool save(Figure8StoredCalibration candidate){
  if(!io_.readSlot||!io_.writeSlot)return false;
  Figure8StoredCalibration a{},b{},old{};
  const bool av=io_.readSlot(0,a)&&figure8CalibrationValid(a);
  const bool bv=io_.readSlot(1,b)&&figure8CalibrationValid(b);
  const bool existing=figure8SelectRecord(av?a:Figure8StoredCalibration{},
                                           bv?b:Figure8StoredCalibration{},old);
  const uint8_t target=!existing?0:(av&&old.sequence==a.sequence?1:0);
  candidate.magic=0x46384341U;candidate.version=1;
  candidate.bytes=sizeof(Figure8StoredCalibration);
  candidate.sequence=existing?old.sequence+1U:1U;
  if(candidate.sequence==0)candidate.sequence=1;
  candidate.crc=figure8RecordCrc(candidate);
  if(!figure8CalibrationValid(candidate))return false;
  if(!io_.writeSlot(target,candidate))return false;
  Figure8StoredCalibration check{};
  return io_.readSlot(target,check)&&figure8CalibrationValid(check)&&
         check.sequence==candidate.sequence&&check.crc==candidate.crc;
 }
private:
 Figure8StoreIo io_;
};
