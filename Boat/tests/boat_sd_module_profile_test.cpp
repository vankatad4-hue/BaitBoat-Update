#include <cassert>
#include "../source/boat_sd_module_profile.h"
int main(){
 BoatSdModuleProfile p{};
 assert(!boatSdModuleProfileValid(p));
 p={true,12,11,13,33,9,4000000U,true,true};
 assert(boatSdModuleProfileValid(p));
 p.cs=9;assert(!boatSdModuleProfileValid(p));
 p.cs=47;assert(!boatSdModuleProfileValid(p));
 p.cs=33;p.independentCsPullupsVerified=false;
 assert(!boatSdModuleProfileValid(p));
 p.independentCsPullupsVerified=true;p.maxClockHz=20000000U;
 assert(!boatSdModuleProfileValid(p));
}
