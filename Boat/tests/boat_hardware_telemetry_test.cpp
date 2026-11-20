#include "../source/boat_hardware_telemetry.h"
#include "../source/radio_payload_encoder.h"
#include <cassert>
#include <iostream>
int main(){
 BoatHardwareSample s={{422000000,275000000,1.25,359.999,true,true,true,true},10,10,10,true,true,true,true,true,true,true,true,false,false,70,3,8,12000,125,10,10,true,true};
 auto b=boatHardware_telemetry(s,11,255,4,MC_RETURNING);
 assert(b.count==4&&b.payload[0].batteryMillivolts==12000&&b.payload[1].headingCentidegrees==35999);
 assert(b.payload[2].coordinateSample==255&&b.payload[3].coordinateSample==255&&b.payload[2].horizontalAccuracyCm==125);
 uint8_t wire[8];RadioDecodedPayload decoded{};
 for(uint8_t i=0;i<b.count;++i){assert(radio_encodeTelemetryPayload(b.payload[i],wire));assert(radio_decodeTelemetryPayload(wire,decoded)&&decoded.kind==b.payload[i].kind);}
 b=boatHardware_telemetry(s,2010,0,255,MC_IDLE);assert(b.count==2&&b.payload[0].batteryMillivolts==65535&&b.payload[1].headingCentidegrees==65535);
 s.fix.latE7=900000001;b=boatHardware_telemetry(s,11,0,255,MC_IDLE);assert(b.count==2);
 std::cout<<"Coherent GNSS, heading and battery telemetry passed\n";
}
