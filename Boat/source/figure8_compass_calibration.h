#pragma once
#include <cmath>
#include <cstdint>
#include "carrier_sensor_math.h"
// Planar calibration only: requires near-level figure-eight turns in BOTH directions.
// Does not estimate vertical soft iron or tilt correction.
struct Figure8Candidate {
 double offsetX,offsetY,scaleX,scaleY;
 uint16_t samples,occupiedSectors;
};
class Figure8CompassCalibration {
public:
 void start(){active_=true;count_=0;sectors_=0;loX_=loY_=INFINITY;hiX_=hiY_=-INFINITY;}
 bool active()const{return active_;}
 bool add(SensorVector m){
  if(!active_||!std::isfinite(m.x)||!std::isfinite(m.y))return false;
  const double r=std::hypot(m.x,m.y);
  if(r<8||r>150)return false;
  if(m.x<loX_)loX_=m.x;if(m.x>hiX_)hiX_=m.x;
  if(m.y<loY_)loY_=m.y;if(m.y>hiY_)hiY_=m.y;
  if(count_<UINT16_MAX)++count_;
  // Occupancy relative to uncorrected origin is a coarse coverage gate.
  const double a=std::atan2(m.y,m.x)+3.141592653589793;
  int sector=(int)(a*(12.0/(2*3.141592653589793)));
  if(sector<0)sector=0;if(sector>11)sector=11;
  sectors_|=(uint16_t)(1u<<sector);
  return true;
 }
 bool finish(Figure8Candidate &out){
  active_=false;out={};
  const double dx=hiX_-loX_,dy=hiY_-loY_;
  if(count_<150||dx<25||dy<25||dx>260||dy>260)return false;
  uint16_t n=0;for(unsigned i=0;i<12;++i)n+=(sectors_>>i)&1u;
  if(n<9)return false;
  const double scale=dx/dy;
  if(scale<0.5||scale>2.0)return false;
  out={ (hiX_+loX_)/2,(hiY_+loY_)/2,(dx+dy)/(2*dx),(dx+dy)/(2*dy),count_,n};
  return true;
 }
private:
 bool active_=false;uint16_t count_=0,sectors_=0;
 double loX_=INFINITY,hiX_=-INFINITY,loY_=INFINITY,hiY_=-INFINITY;
};
