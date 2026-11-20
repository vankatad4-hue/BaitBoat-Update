#pragma once
#include <cmath>
#include <stdint.h>
struct SensorVector{double x,y,z;};
inline double sensorDot(SensorVector a,SensorVector b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline SensorVector sensorScale(SensorVector a,double s){return {a.x*s,a.y*s,a.z*s};}
inline SensorVector sensorSub(SensorVector a,SensorVector b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline SensorVector sensorCross(SensorVector a,SensorVector b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
struct CarrierHeadingCalibration{
 bool commissioned;double magOffset[3],magMatrix[9],accelMatrix[9],declinationDeg;
 double minimumFieldUt,maximumFieldUt;
};
inline SensorVector sensorMatrix(const double *a,SensorVector v){return {a[0]*v.x+a[1]*v.y+a[2]*v.z,a[3]*v.x+a[4]*v.y+a[5]*v.z,a[6]*v.x+a[7]*v.y+a[8]*v.z};}
inline bool carrierHeading(SensorVector accelG,SensorVector magUt,const CarrierHeadingCalibration &c,double &deg){
 if(!c.commissioned||!std::isfinite(c.declinationDeg)||std::fabs(c.declinationDeg)>180||!std::isfinite(c.minimumFieldUt)||!std::isfinite(c.maximumFieldUt)||c.minimumFieldUt<=0||c.maximumFieldUt<=c.minimumFieldUt)return false;
 // Calibrated body coordinates: forward/right/down. Accelerometer at rest
 // measures specific force opposite gravity; gravity direction is -accel.
 auto a=sensorMatrix(c.accelMatrix,accelG);auto m=sensorMatrix(c.magMatrix,sensorSub(magUt,{c.magOffset[0],c.magOffset[1],c.magOffset[2]}));
 double g=std::sqrt(sensorDot(a,a)),field=std::sqrt(sensorDot(m,m));
 if(!std::isfinite(g)||!std::isfinite(field)||g<0.8||g>1.2||field<c.minimumFieldUt||field>c.maximumFieldUt)return false;
 const auto down=sensorScale(a,-1/g);
 if(down.z<0.8)return false;
 auto forward=sensorSub({1,0,0},sensorScale(down,down.x));double fn=std::sqrt(sensorDot(forward,forward));if(fn<0.1)return false;
 forward=sensorScale(forward,1/fn);const auto right=sensorCross(down,forward);
 const double x=sensorDot(m,forward),y=sensorDot(m,right);
 if(std::hypot(x,y)<5)return false;
 deg=std::fmod(std::atan2(-y,x)*180/3.141592653589793+c.declinationDeg+360,360);return std::isfinite(deg);
}
struct CarrierBatteryCalibration{bool commissioned;double dividerRatio,gain,offsetMv;uint16_t minimumMv,maximumMv;};
inline bool carrierBatteryMv(int16_t adc,const CarrierBatteryCalibration &c,uint16_t &mv){
 if(!c.commissioned||!std::isfinite(c.dividerRatio)||!std::isfinite(c.gain)||!std::isfinite(c.offsetMv)||c.dividerRatio<=1||c.dividerRatio>20||c.gain<0.9||c.gain>1.1||std::fabs(c.offsetMv)>500||!c.minimumMv||c.maximumMv<=c.minimumMv)return false;
 double input=adc*0.125; // ADS1115 +/-4.096 V full scale
 if(input<=0||input>3000)return false;
 const double value=input*c.dividerRatio*c.gain+c.offsetMv;
 if(value<c.minimumMv||value>c.maximumMv)return false;
 mv=(uint16_t)std::lround(value);return true;
}
