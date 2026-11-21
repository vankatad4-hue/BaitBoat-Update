#pragma once
#include <cmath>
#include <stdint.h>
#include "carrier_sensor_math.h"
// Non-actuating, RAM-only magnetometer sweep. A candidate is NOT commissioned
// until independently reviewed and installed in the signed sensor profile.
struct CompassCalibrationCandidate {
 double offsetUt[3],scale[3];uint32_t samples;double spanUt[3];
};
class CompassCalibrationSession {
public:
 void start(){active_=true;count_=0;for(int i=0;i<3;++i){lo_[i]=INFINITY;hi_[i]=-INFINITY;}}
 bool active()const{return active_;}
 bool add(SensorVector m){
  if(!active_||!std::isfinite(m.x)||!std::isfinite(m.y)||!std::isfinite(m.z))return false;
  const double v[3]={m.x,m.y,m.z};
  if(std::hypot(std::hypot(v[0],v[1]),v[2])<10||std::hypot(std::hypot(v[0],v[1]),v[2])>150)return false;
  for(int i=0;i<3;++i){if(v[i]<lo_[i])lo_[i]=v[i];if(v[i]>hi_[i])hi_[i]=v[i];}
  if(count_<UINT32_MAX)++count_;
  return true;
 }
 bool finish(CompassCalibrationCandidate &out){
  active_=false;out={};
  if(count_<150)return false;
  double half[3],sum=0;
  for(int i=0;i<3;++i){
   const double span=hi_[i]-lo_[i];
   if(!std::isfinite(span)||span<20||span>240)return false;
   half[i]=span/2;sum+=half[i];
  }
  const double mean=sum/3;
  for(int i=0;i<3;++i){
   const double factor=mean/half[i];
   if(factor<0.5||factor>2.0)return false;
   out.offsetUt[i]=(hi_[i]+lo_[i])/2;
   out.scale[i]=factor;out.spanUt[i]=hi_[i]-lo_[i];
  }
  out.samples=count_;return true;
 }
private:
 bool active_=false;uint32_t count_=0;
 double lo_[3]={},hi_[3]={};
};
