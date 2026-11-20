#include "../source/tracker_gnss.h"
#include "../source/carrier_sensor_math.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <iostream>
static void feed(TrackerGnss &g,const std::string &s,uint32_t at){
 uint8_t xorSum=0;for(char c:s)xorSum^=(uint8_t)c;
 char tail[8];std::snprintf(tail,sizeof(tail),"*%02X\r\n",(unsigned)xorSum);
 for(char c:("$"+s+tail))g.feed(c,at);
}
int main(){
 TrackerGnss g;
 feed(g,"GNGGA,120000.00,4212.0000,N,02730.0000,E,1,08,1.0,0,M,0,M,,",10);
 feed(g,"GNGSA,A,3",11);feed(g,"GNRMC,120000.00,A,4212.0000,N,02730.0000,E,2.0",12);
 assert(!g.snapshot(12).positionValid); // no accuracy measurement
 feed(g,"GNGST,120001.00,1,1,1,0,0.3,0.4,1",13);assert(!g.snapshot(13).positionValid);
 feed(g,"GNGST,120000.00,1,1,1,0,0.3,0.4,1",14);
 auto f=g.snapshot(14);assert(f.positionValid&&f.latE7==422000000&&f.lonE7==275000000&&f.speedCmS==103&&std::fabs(f.estimatedAccuracyM-1)<1e-9);
 assert(!g.snapshot(2010).positionValid);
 feed(g,"GNGGA,120000.00,4212.0000,N,02730.0000,E,1,08,1.0",2011);assert(!g.snapshot(2011).positionValid);
 for(char c:std::string("$GNGGA,120002,4212.0,N,02730.0,E,1,08*00\r\n"))g.feed(c,2020);
 assert(!g.snapshot(2020).positionValid);
 CarrierHeadingCalibration c={true,{0,0,0},{1,0,0,0,1,0,0,0,1},{1,0,0,0,1,0,0,0,1},0,15,80};
 double heading=0;assert(carrierHeading({0,0,-1},{0,-40,20},c,heading)&&std::fabs(heading-90)<1e-8);
 assert(!carrierHeading({0,0,-1},{0,-100,20},c,heading));
 c.commissioned=false;assert(!carrierHeading({0,0,-1},{0,-40,20},c,heading));
 CarrierBatteryCalibration b={true,5.7,1,0,8000,14000};uint16_t mv=0;
 assert(carrierBatteryMv(17684,b,mv)&&mv>=12599&&mv<=12601);
 assert(!carrierBatteryMv(-1,b,mv));b.commissioned=false;assert(!carrierBatteryMv(17684,b,mv));
 std::cout<<"Tracker GNSS and calibrated sensor math passed\n";
}
