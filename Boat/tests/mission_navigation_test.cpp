#include "../source/mission_navigation.h"
#include <cassert>
#include <iostream>
#include <limits>
int main(){
 MissionPlan p={1,0,0,0,300,0,1000,0,0};
 MissionNavigationFix f={0,0,1,90,true,true,true,true};
 auto r=missionNavigation_evaluate(p,f,false);
 assert(r.valid&&!r.arrived&&r.distanceM>11&&r.distanceM<12&&std::fabs(r.headingErrorDeg)<0.001);
 r=missionNavigation_evaluate(p,f,true);assert(r.valid&&r.arrived);
 f.lonE7=1000;assert(missionNavigation_evaluate(p,f,false).arrived);
 f.accuracyM=3.1;assert(!missionNavigation_evaluate(p,f,false).arrived);
 f.accuracyM=std::numeric_limits<double>::quiet_NaN();assert(!missionNavigation_evaluate(p,f,false).valid);
 f.accuracyM=1;f.fresh=false;assert(!missionNavigation_evaluate(p,f,false).valid);
 f.fresh=true;f.geofenceValid=false;assert(!missionNavigation_evaluate(p,f,false).valid);
 f.geofenceValid=true;f.headingValid=false;assert(!missionNavigation_evaluate(p,f,false).valid);
 f.headingValid=true;f.batteryHealthy=false;assert(!missionNavigation_evaluate(p,f,false).valid);
 f.batteryHealthy=true;f.lonE7=1799999900;p.targetLonE7=-1799999900;
 r=missionNavigation_evaluate(p,f,false);assert(r.valid&&r.distanceM>2&&r.distanceM<3&&!r.arrived);
 std::cout<<"Mission navigation geometry and arrival gate passed\n";
}
