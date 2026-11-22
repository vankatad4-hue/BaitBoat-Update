#pragma once
#include <stdint.h>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "gnss_utc_sync.h"
// UC6580 NMEA reader: bounded buffer, checksum, same UTC epoch for
// GGA/RMC/GST, recent GSA fix dimension. No HDOP-to-metres fabrication.
struct TrackerFix{int32_t latE7,lonE7;double estimatedAccuracyM;uint16_t speedCmS;uint8_t fixType,satellites;uint32_t at;bool positionValid,speedValid;};
class TrackerGnss{
public:
 void feed(char c,uint32_t now){
  if(c=='$'){length_=0;collect_=true;return;}
  if(!collect_)return;
  if(c=='\r')return;
  if(c=='\n'){line_[length_]=0;collect_=false;parse(now);return;}
  if(c<32||c>126||length_>=sizeof(line_)-1){collect_=false;return;}
  line_[length_++]=c;
 }
 flytec_utc::Sample utcSnapshot(uint32_t now)const{
  auto s=utc_;
  if(!rmcValid_||!positionValid_||!gsaValid_||fixType_!=3||
     positionUtc_!=rmcUtc_||
     (uint32_t)(now-gsaAt_)>=2000U||(uint32_t)(now-positionAt_)>=2000U||
     !flytec_utc::fresh(s,now))s.valid=false;
  return s;
 }
 TrackerFix snapshot(uint32_t now)const{
  TrackerFix out{};out.at=positionAt_;out.latE7=lat_;out.lonE7=lon_;out.satellites=sats_;
  out.fixType=gsaValid_&&now-gsaAt_<2000U?fixType_:0;
  if(!positionValid_||!rmcValid_||!gstValid_||out.fixType!=3||now-positionAt_>=2000U||now-rmcAt_>=2000U||now-gstAt_>=2000U||positionUtc_!=rmcUtc_||positionUtc_!=gstUtc_)return out;
  out.estimatedAccuracyM=accuracy_;out.positionValid=true;out.speedCmS=speed_;out.speedValid=true;return out;
 }
private:
 char line_[128]={};size_t length_=0;bool collect_=false;
 bool positionValid_=false,rmcValid_=false,gstValid_=false,gsaValid_=false,positionSeen_=false,rmcSeen_=false,gstSeen_=false;
 uint32_t positionUtc_=0,rmcUtc_=0,gstUtc_=0,positionAt_=0,rmcAt_=0,gstAt_=0,gsaAt_=0;
 uint8_t fixType_=0,sats_=0;int32_t lat_=0,lon_=0;double accuracy_=0;uint16_t speed_=0;
 flytec_utc::Sample utc_{};
 static int hex(char c){return c>='0'&&c<='9'?c-'0':c>='A'&&c<='F'?c-'A'+10:c>='a'&&c<='f'?c-'a'+10:-1;}
 static bool number(const char *s,double &v){
  if(!s||!*s)return false;
  char *end=nullptr;v=std::strtod(s,&end);return end&&!*end&&std::isfinite(v);
 }
 static bool utc(const char *s,uint32_t &v){
  double d;if(!number(s,d)||d<0||d>=240000)return false;
  unsigned h=(unsigned)(d/10000),m=(unsigned)(d/100)%100;double sec=std::fmod(d,100.0);
  if(h>=24||m>=60||sec>=60)return false;
  v=(uint32_t)std::llround((h*3600+m*60+sec)*1000);return v<86400000;
 }
 static bool coordinate(const char *s,const char *hem,bool lat,int32_t &out){
  double raw;if(!number(s,raw)||raw<0||!hem||strlen(hem)!=1)return false;
  const double deg=std::floor(raw/100),minutes=raw-deg*100;
  if(minutes>=60||deg+minutes/60>(lat?90:180))return false;
  const char pos=lat?'N':'E',neg=lat?'S':'W';if(*hem!=pos&&*hem!=neg)return false;
  out=(int32_t)std::llround((deg+minutes/60)*1e7*(*hem==neg?-1:1));return true;
 }
 void parse(uint32_t now){
  char *star=strchr(line_,'*');if(!star||strlen(star)!=3)return;
  int a=hex(star[1]),b=hex(star[2]);if(a<0||b<0)return;
  uint8_t sum=0;for(char *p=line_;p<star;++p)sum^=(uint8_t)*p;
  if(sum!=(uint8_t)(a*16+b))return;
  *star=0;char *f[24]={line_};unsigned n=1;
  for(char *p=line_;*p;++p)if(*p==','){*p=0;if(n>=24)return;f[n++]=p+1;}
  if(strlen(f[0])!=5)return;
  const char *kind=f[0]+2;uint32_t epoch=0;double d=0;
  if(!strcmp(kind,"GGA")){
   if(n<8||!utc(f[1],epoch)||!number(f[6],d)||!(d==1||d==2||d==4||d==5)){positionValid_=false;return;}
   int32_t lat,lon;double sats;
   if(!coordinate(f[2],f[3],true,lat)||!coordinate(f[4],f[5],false,lon)||!number(f[7],sats)||sats<4||sats>99||sats!=std::floor(sats)){positionValid_=false;return;}
   if(positionSeen_&&positionUtc_==epoch)return;
   positionSeen_=positionValid_=true;positionUtc_=epoch;positionAt_=now;lat_=lat;lon_=lon;sats_=(uint8_t)sats;
  }else if(!strcmp(kind,"GSA")){
   if(n<3||!number(f[2],d)||d!=3){gsaValid_=false;return;}
   gsaValid_=true;fixType_=3;gsaAt_=now;
  }else if(!strcmp(kind,"RMC")){
   if(n<8||!utc(f[1],epoch)||strcmp(f[2],"A")||!number(f[7],d)||d<0||d>194.0){
    rmcValid_=false;utc_.valid=false;return;
   }
   // Preserve existing GNSS position/speed parsing for legacy RMC without
   // a UTC date. Such sentences NEVER qualify the wall-clock discipline.
   uint64_t unixMs=0;
   const bool utcDateValid=n>=10&&flytec_utc::utcFromNmeaDate(f[9],epoch,unixMs);
   if(rmcSeen_&&rmcUtc_==epoch)return;
   rmcSeen_=rmcValid_=true;rmcUtc_=epoch;rmcAt_=now;speed_=(uint16_t)std::llround(d*51.444444);
   // NMEA sentence-end timestamp: serial buffering latency unknown and
   // no PPS anchor. Time is COARSE only and NOT stop-latency evidence.
   utc_.unixMs=utcDateValid?unixMs:0;
   utc_.localMs=now;utc_.receiverTimeAccMs=1000U;utc_.valid=utcDateValid;
  }else if(!strcmp(kind,"GST")){
   double latSigma,lonSigma;
   if(n<9||!utc(f[1],epoch)||!number(f[6],latSigma)||!number(f[7],lonSigma)||latSigma<=0||lonSigma<=0){gstValid_=false;return;}
   // Provisional conservative error estimate; not a certified protection level.
   const double estimate=2*std::hypot(latSigma,lonSigma);
   if(!std::isfinite(estimate)||estimate>655.34){gstValid_=false;return;}
   if(gstSeen_&&gstUtc_==epoch)return;
   gstSeen_=gstValid_=true;gstUtc_=epoch;gstAt_=now;accuracy_=estimate;
  }
 }
};
