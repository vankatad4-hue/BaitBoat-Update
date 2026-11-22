#include "../source/boat_home_point_record.h"
#include <cassert>
#include <cstdint>
#include <iostream>

int main(){
 BoatHomePoint original={422000000,275000000,7};
 uint8_t bytes[BOAT_HOME_POINT_RECORD_BYTES]={};
 assert(boatHomeRecordEncode(original,bytes));
 BoatHomePoint decoded{};
 assert(boatHomeRecordDecode(bytes,decoded));
 assert(boatHomePointEqual(original,decoded));

 // The CRC rejects torn/corrupt NVS bytes and never returns a partial point.
 uint8_t corrupt[BOAT_HOME_POINT_RECORD_BYTES];
 memcpy(corrupt,bytes,sizeof(corrupt));
 corrupt[11]^=0x20U;
 assert(!boatHomeRecordDecode(corrupt,decoded));

 // Header/version/length, coordinate range and sequence are all validated.
 memcpy(corrupt,bytes,sizeof(corrupt));corrupt[4]=2;
 assert(!boatHomeRecordDecode(corrupt,decoded));
 memcpy(corrupt,bytes,sizeof(corrupt));corrupt[6]=23;
 assert(!boatHomeRecordDecode(corrupt,decoded));
 BoatHomePoint invalidLat={900000001,275000000,1};
 assert(!boatHomeRecordEncode(invalidLat,bytes));
 BoatHomePoint invalidLon={422000000,1800000001,1};
 assert(!boatHomeRecordEncode(invalidLon,bytes));
 BoatHomePoint noSequence={422000000,275000000,0};
 assert(!boatHomeRecordEncode(noSequence,bytes));

 assert(boatHomeSequenceNewer(8,7));
 assert(!boatHomeSequenceNewer(7,8));
 assert(boatHomeSequenceNewer(1,0xFFFFFFFFU)); // wrap-safe ordering
 std::cout<<"Boat Home NVS record CRC/range tests passed\\n";
}
