#pragma once
#include "mission_protocol.h"
#include <cmath>
// Pure navigation geometry. Deliberately no actuator output or mission-start API.
struct MissionNavigationFix{
 int32_t latE7,lonE7;
 double accuracyM,headingDeg;
 bool fresh,headingValid,batteryHealthy,geofenceValid;
};
struct MissionNavigationResult{
 bool valid,arrived;
 double distanceM,bearingDeg,headingErrorDeg;
};
inline MissionNavigationResult missionNavigation_evaluate(const MissionPlan &plan,
 const MissionNavigationFix &fix,bool goingHome){
 MissionNavigationResult r{};
 if(!missionPlan_valid(plan)||!fix.fresh||!fix.headingValid||!fix.batteryHealthy||!fix.geofenceValid||
    fix.latE7 < -900000000||fix.latE7 > 900000000||
    fix.lonE7 < -1800000000||fix.lonE7 > 1800000000||
    !std::isfinite(fix.accuracyM)||fix.accuracyM<=0||
    !std::isfinite(fix.headingDeg)||fix.headingDeg<0||fix.headingDeg>=360)return r;
 const double rad=0.017453292519943295,metres=6371000.0;
 const int32_t targetLat=goingHome?plan.homeLatE7:plan.targetLatE7;
 const int32_t targetLon=goingHome?plan.homeLonE7:plan.targetLonE7;
 const double lat1=fix.latE7*1e-7*rad,lat2=targetLat*1e-7*rad;
 const double dlat=lat2-lat1,dlon=std::remainder(((double)targetLon-fix.lonE7)*1e-7*rad,360*rad);
 double a=std::sin(dlat/2)*std::sin(dlat/2)+std::cos(lat1)*std::cos(lat2)*std::sin(dlon/2)*std::sin(dlon/2);
 a=std::fmax(0.0,std::fmin(1.0,a));
 r.distanceM=2*metres*std::atan2(std::sqrt(a),std::sqrt(1-a));
 const double y=std::sin(dlon)*std::cos(lat2);
 const double x=std::cos(lat1)*std::sin(lat2)-std::sin(lat1)*std::cos(lat2)*std::cos(dlon);
 r.bearingDeg=std::fmod(std::atan2(y,x)/rad+360,360);
 r.headingErrorDeg=std::remainder(r.bearingDeg-fix.headingDeg,360.0);
 r.valid=true;
 // Conservative arrival: estimated horizontal-error circle must fit in radius.
 r.arrived=r.distanceM+fix.accuracyM<=plan.radiusCm/100.0;
 if(r.arrived){r.headingErrorDeg=0;} // no steering at qualified arrival
 return r;
}
