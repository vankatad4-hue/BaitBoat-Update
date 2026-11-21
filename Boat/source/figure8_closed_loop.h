#pragma once
#include <stdint.h>
#include <cmath>
#include "figure8_autonomous_motion.h"
// Bounded heading-feedback figure-eight supervisor. NOT a collision-avoidance
// system; requires an independently surveyed geofence and physical power cutoff.
// Heading in degrees clockwise from north. No steering without fresh heading.
struct Figure8NavSample {
 double headingDeg=0;
 bool headingFresh=false;
 double speedMps=0;
 bool speedFresh=false;
};
class Figure8ClosedLoop {
public:
 bool start(uint32_t now,const Figure8MotionSafety &s,const Figure8NavSample &nav){
  if(!nav.headingFresh||!nav.speedFresh||!std::isfinite(nav.headingDeg)||!std::isfinite(nav.speedMps)||
     nav.speedMps>0.15||!motion_.start(now,s))return false;
  startHeading_=normalize(nav.headingDeg);lastHeading_=startHeading_;
  signedTurn_=0;last_=now;leg_=0;return true;
 }
 Figure8MotorIntent tick(uint32_t now,const Figure8MotionSafety &s,const Figure8NavSample &nav,Figure8AutoState cal){
  const auto safety=motion_.tick(now,s,cal);
  if(!safety.valid)return {};
  if(!nav.headingFresh||!nav.speedFresh||!std::isfinite(nav.headingDeg)||
     !std::isfinite(nav.speedMps)||nav.speedMps<0.15||nav.speedMps>1.5||
     (uint32_t)(now-last_)>250U){motion_.abort();return {};}
  const double h=normalize(nav.headingDeg);
  double step=h-lastHeading_;
  if(step>180)step-=360;
  if(step< -180)step+=360;
  // Reject discontinuous magnetometer jumps; use independent gyro/GNSS
  // agreement in the commissioned adapter before setting headingFresh.
  if(std::fabs(step)>20.0){motion_.abort();return {};}
  signedTurn_+=step;lastHeading_=h;last_=now;
  const double direction=leg_==0?1.0:-1.0;
  if(direction*signedTurn_>=350.0){
   if(leg_==0){leg_=1;signedTurn_=0;}
   else {motion_.abort();return {};}// End of two complete opposing loops.
  }
  // Forward-only differential thrust; heading feedback switches turn direction
  // only after measured angular progress. This is NOT radius/track control.
  // Actual PWM/current limits remain
  // with the physical driver. No unsafe zero-radius pivot.
  const bool clockwise=leg_==0;
  return clockwise?Figure8MotorIntent{180,100,true}:Figure8MotorIntent{100,180,true};
 }
 void abort(){motion_.abort();}
 Figure8MotionState state()const{return motion_.state();}
 uint8_t leg()const{return leg_;}
private:
 static double normalize(double h){h=std::fmod(h,360.0);return h<0?h+360.0:h;}
 Figure8AutonomousMotion motion_{};
 double startHeading_=0,lastHeading_=0,signedTurn_=0;
 uint32_t last_=0;uint8_t leg_=0;
};
