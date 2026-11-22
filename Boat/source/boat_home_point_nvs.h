#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "boat_home_point_record.h"

// Two-slot NVS store: write the inactive slot, read it back, and keep the
// previous CRC-valid Home record if power is interrupted.
class BoatHomePointNvs {
public:
 bool begin(){
  const bool ok=prefs_.begin("boat_home",false);
  if(ok)instance_=this;
  return ok;
 }
 void end(){
  prefs_.end();
  if(instance_==this)instance_=nullptr;
 }
 bool load(BoatHomePoint &out){
  BoatHomePoint a{},b{};
  const bool av=readSlot(0,a),bv=readSlot(1,b);
  if(!av&&!bv)return false;
  out=av&&(!bv||boatHomeSequenceNewer(a.sequence,b.sequence))?a:b;
  return true;
 }
 bool save(const BoatHomePoint &input){
  if(!boatHomeCoordinatesValid(input.latE7,input.lonE7))return false;
  BoatHomePoint a{},b{};
  const bool av=readSlot(0,a),bv=readSlot(1,b);
  uint8_t target=0;
  uint32_t next=1;
  if(av&&bv){
   if(boatHomeSequenceNewer(a.sequence,b.sequence)){target=1;next=a.sequence+1U;}
   else {target=0;next=b.sequence+1U;}
  }else if(av){target=1;next=a.sequence+1U;}
  else if(bv){target=0;next=b.sequence+1U;}
  if(!next)next=1;
  const BoatHomePoint value={input.latE7,input.lonE7,next};
  return writeSlot(target,value);
 }
 static bool loadBridge(BoatHomePoint &out){return instance_&&instance_->load(out);}
 static bool saveBridge(const BoatHomePoint &point){return instance_&&instance_->save(point);}
private:
 Preferences prefs_{};
 static BoatHomePointNvs *instance_;
 bool readSlot(uint8_t slot,BoatHomePoint &out){
  if(slot>1)return false;
  const char *key=slot==0?"home_a":"home_b";
  if(prefs_.getBytesLength(key)!=BOAT_HOME_POINT_RECORD_BYTES)return false;
  uint8_t raw[BOAT_HOME_POINT_RECORD_BYTES]={};
  if(prefs_.getBytes(key,raw,sizeof(raw))!=sizeof(raw))return false;
  return boatHomeRecordDecode(raw,out);
 }
 bool writeSlot(uint8_t slot,const BoatHomePoint &point){
  if(slot>1)return false;
  const char *key=slot==0?"home_a":"home_b";
  uint8_t raw[BOAT_HOME_POINT_RECORD_BYTES]={};
  if(!boatHomeRecordEncode(point,raw))return false;
  if(prefs_.putBytes(key,raw,sizeof(raw))!=sizeof(raw))return false;
  BoatHomePoint verify{};
  return readSlot(slot,verify)&&boatHomePointEqual(point,verify);
 }
};
