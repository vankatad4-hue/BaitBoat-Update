#include "../source/gnss_utc_sync.h"
#include "../source/radio_utc_time_v1.h"
#include "../source/m100_ubx_nav_pvt.h"
#include "../source/tracker_gnss.h"
#include <assert.h>
#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string.h>

static void feedNmea(TrackerGnss &g,const std::string &body,uint32_t t){
 uint8_t cs=0;for(char c:body)cs^=(uint8_t)c;
 char end[8];snprintf(end,sizeof(end),"*%02X\r\n",(unsigned)cs);
 const std::string frame="$"+body+end;
 for(char c:frame)g.feed(c,t);
}
static void feedUbx(M100UbxNavPvt &g,uint32_t t,bool validTime=true,
                    bool validPos=true,uint32_t tAccNs=5000000U){
 uint8_t p[92]{};
 p[4]=0xEA;p[5]=0x07; // 2026
 p[6]=11;p[7]=22;p[8]=12;p[9]=34;p[10]=56;p[11]=validTime?7:0;
 auto put=[](uint8_t *dst,uint32_t v){
   for(int i=0;i<4;++i)dst[i]=(uint8_t)(v>>(i*8));
 };
 put(p+12,tAccNs);put(p+16,123000000U);
 p[20]=validPos?3:1;p[21]=1;p[23]=12;
 put(p+24,271234567);put(p+28,431234567);put(p+40,1500);put(p+60,2300);
 uint8_t ckA=0,ckB=0;
 auto add=[&](uint8_t b){ckA=(uint8_t)(ckA+b);ckB=(uint8_t)(ckB+ckA);g.feed(b,t);};
 g.feed(0xB5,t);g.feed(0x62,t);
 add(0x01);add(0x07);add(92);add(0);
 for(auto b:p)add(b);
 g.feed(ckA,t);g.feed(ckB,t);
}
int main(){
 using namespace flytec_utc;
 using namespace flytec_time;
 uint64_t expected=0;
 assert(unixFromCivil(2026,11,22,12,34,56,123,expected));
 assert(expected>=MIN_UNIX_MS&&expected<MAX_UNIX_MS);
 uint64_t leapDay=0;
 assert(unixFromCivil(2024,2,29,0,0,0,0,leapDay));
 assert(leapDay==1709164800000ULL);
 assert(!unixFromCivil(2025,2,29,12,0,0,0,leapDay));
 assert(!unixFromCivil(2100,1,1,0,0,0,0,leapDay));
 assert(!unixFromCivil(2026,11,22,23,59,60,0,leapDay));
 assert(!unixFromCivil(2026,13,1,0,0,0,0,leapDay));
 assert(!utcFromNmeaDate("311126",12*3600000UL,leapDay));
 assert(utcFromNmeaDate("221126",12*3600000UL+34*60000UL+56*1000UL+123,leapDay));
 assert(leapDay==expected);
 uint32_t acc=0;
 uint64_t ubxMs=0;
 assert(utcFromUbx(2026,11,22,12,34,56,123000000,5000000U,0x07,ubxMs,acc));
 assert(ubxMs==expected&&acc==5);
 assert(!utcFromUbx(2026,11,22,12,34,56,123000000,251000000U,7,ubxMs,acc));
 assert(!utcFromUbx(2026,11,22,12,34,56,123000000,5000000U,3,ubxMs,acc));
 assert(utcFromUbx(2026,11,22,12,34,56,-250000000,5000000U,7,ubxMs,acc));
 assert(ubxMs==expected-373ULL); // negative nano rolls into previous second
 M100UbxNavPvt parser;
 feedUbx(parser,1000);
 auto ubx=parser.snapshot(1050);
 assert(ubx.positionValid&&ubx.utc.valid&&ubx.utc.unixMs==expected);
 assert(ubx.utc.receiverTimeAccMs==5U);
 assert(!parser.snapshot(3100).utc.valid);
 feedUbx(parser,3200,false);
 assert(!parser.snapshot(3200).utc.valid);
 feedUbx(parser,4000,true,true,250000001U);
 assert(!parser.snapshot(4000).utc.valid);
 feedUbx(parser,5000,true,false);
 assert(!parser.snapshot(5000).utc.valid);
 TrackerGnss nmea;
 feedNmea(nmea,"GNGGA,123456.123,4212.0000,N,02730.0000,E,1,08,1.0,0,M,0,M,,",10);
 feedNmea(nmea,"GNGSA,A,3",11);
 feedNmea(nmea,"GNRMC,123456.123,A,4212.0000,N,02730.0000,E,2.0,0.0,221126,,,A",12);
 auto sample=nmea.utcSnapshot(12);
 assert(sample.valid&&sample.unixMs==expected);
 assert(!nmea.utcSnapshot(2013).valid);
 TrackerGnss legacy;
 feedNmea(legacy,"GNGGA,123456.123,4212.0000,N,02730.0000,E,1,08,1.0,0,M,0,M,,",10);
 feedNmea(legacy,"GNGSA,A,3",11);
 feedNmea(legacy,"GNRMC,123456.123,A,4212.0000,N,02730.0000,E,2.0",12);
 assert(!legacy.utcSnapshot(12).valid);
 // Valid legacy navigation fields are NOT disabled by missing UTC date.
 feedNmea(legacy,"GNGST,123456.123,1,1,1,0,0.3,0.4,1",13);
 assert(legacy.snapshot(13).positionValid);
 uint8_t wire[8]{};
 assert(encode(expected,wire)&&wire[0]==9&&wire[1]==1);
 uint64_t decoded=0;
 assert(decode(wire,decoded)&&decoded==expected);
 wire[1]=2;assert(!decode(wire,decoded));wire[1]=1;
 wire[6]=0xE8;wire[7]=0x03;assert(!decode(wire,decoded)); // 1000 ms
 assert(!encode(123,wire));
 Discipline boatClock;
 assert(boatClock.observe(expected,1000));
 assert(!boatClock.utcNow(1000,decoded));
 assert(boatClock.observe(expected+1000,2000));
 assert(boatClock.utcNow(2010,decoded)&&decoded==expected+1010ULL);
 assert(boatClock.shouldSetRtc(2010));
 boatClock.markRtcSet(2010);
 assert(!boatClock.shouldSetRtc(3000));
 assert(!boatClock.utcNow(25001,decoded)); // stale >20s
 assert(!boatClock.observe(expected-100000,3000)); // spoof/backwards jump
 assert(!boatClock.qualified());
 Discipline wrapClock;
 assert(wrapClock.observe(expected,0xFFFFFFF0U));
 assert(wrapClock.observe(expected+48,0x20U));
 assert(wrapClock.utcNow(0x21U,decoded)&&decoded==expected+49ULL);
 puts("R30H-5 GNSS UTC UBX/NMEA/wire/RTC discipline tests passed (NO_GPS_PPS_NO_RF)");
 return 0;
}
