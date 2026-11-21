#include <cassert>
#include "../source/boat_wifi_ota.h"
int main(){
 BoatOtaPreflight s{};
 assert(!boatOtaPreflightOK(s));
 s={true,true,true,true,true,true,true,true,true,true,true};
 assert(boatOtaPreflightOK(s));
 s.motorPowerIsolated=false;assert(!boatOtaPreflightOK(s));
 s.motorPowerIsolated=true;s.solenoidPowerIsolated=false;assert(!boatOtaPreflightOK(s));
 s.solenoidPowerIsolated=true;s.secureBootEnabled=false;assert(!boatOtaPreflightOK(s));
 s.secureBootEnabled=true;s.physicalMaintenanceSwitch=false;assert(!boatOtaPreflightOK(s));
 s.physicalMaintenanceSwitch=true;s.operatorConfirmed=false;assert(!boatOtaPreflightOK(s));
}
