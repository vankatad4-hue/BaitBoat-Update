#include <cassert>
#include <cstdint>
#include "../source/m100_ubx_nav_pvt.h"
static void frame(M100UbxNavPvt &p,uint32_t now,bool corrupt=false){
 uint8_t b[92]={};b[20]=3;b[21]=1;b[23]=12;
 auto put=[&](int i,uint32_t v){for(int k=0;k<4;++k)b[i+k]=(uint8_t)(v>>(8*k));};
 put(24,(uint32_t)271234567);put(28,(uint32_t)431234567);put(40,1500);put(60,2300);
 uint8_t a=0,c=0;auto add=[&](uint8_t v){a=(uint8_t)(a+v);c=(uint8_t)(c+a);p.feed(v,now);};
 p.feed(0xb5,now);p.feed(0x62,now);
 add(1);add(7);add(92);add(0);
 for(auto v:b)add(v);
 p.feed(corrupt?(uint8_t)(a^1):a,now);p.feed(c,now);
}
int main(){
 M100UbxNavPvt p;
 frame(p,1000,true);assert(!p.snapshot(1000).positionValid);
 frame(p,2000);
 auto f=p.snapshot(2100);assert(f.positionValid&&f.speedValid);
 assert(f.latE7==431234567&&f.lonE7==271234567);
 assert(f.satellites==12&&f.accuracyM==1.5&&f.speedCmS==230);
 assert(!p.snapshot(4100).positionValid);
}
