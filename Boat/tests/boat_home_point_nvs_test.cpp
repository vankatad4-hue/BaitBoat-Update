#include "../source/boat_home_point_nvs.h"
#include <cassert>
#include <iostream>

int main(){
 Preferences::clearStore();
 BoatHomePointNvs nvs;
 assert(nvs.begin());
 BoatHomePoint first={422000000,275000000,0};
 assert(nvs.save(first));
 BoatHomePoint loaded{};
 assert(nvs.load(loaded));
 assert(loaded.latE7==first.latE7&&loaded.lonE7==first.lonE7&&loaded.sequence==1U);

 BoatHomePoint second={423000000,276000000,0};
 assert(nvs.save(second));
 assert(nvs.load(loaded));
 assert(loaded.latE7==second.latE7&&loaded.lonE7==second.lonE7&&loaded.sequence==2U);

 // Corrupt the newest slot: the previous CRC-valid slot must remain usable.
 assert(Preferences::corrupt("home_b",10));
 assert(nvs.load(loaded));
 assert(loaded.latE7==first.latE7&&loaded.lonE7==first.lonE7&&loaded.sequence==1U);
 nvs.end();

 // Reconstructing the adapter models a controller power-cycle; the older
 // valid Home survives, and a new write safely replaces the damaged slot.
 BoatHomePointNvs rebooted;
 assert(rebooted.begin());
 assert(rebooted.load(loaded)&&loaded.sequence==1U);
 BoatHomePoint third={424000000,277000000,0};
 assert(rebooted.save(third));
 assert(rebooted.load(loaded));
 assert(loaded.latE7==third.latE7&&loaded.lonE7==third.lonE7&&loaded.sequence==2U);
 assert(BoatHomePointNvs::loadBridge(loaded));
 BoatHomePoint fourth={425000000,278000000,0};
 assert(BoatHomePointNvs::saveBridge(fourth));
 assert(rebooted.load(loaded)&&loaded.latE7==fourth.latE7&&loaded.sequence==3U);
 BoatHomePoint invalid={900000001,0,0};
 assert(!rebooted.save(invalid));
 rebooted.end();

 Preferences::clearStore();
 BoatHomePointNvs empty;
 assert(empty.begin());
 assert(!empty.load(loaded));
 empty.end();
 std::cout<<"Boat Home two-slot NVS adapter tests passed\\n";
}
