#include "../source/mission_stick.h"
#include <cassert>
#include <iostream>
int main(){
 MissionStickInput s={7,100,-200,false},decoded{};uint8_t p[8];int16_t l=1,r=1;MissionStickLease lease;
 assert(missionStickEncode(s,p)&&missionStickDecode(p,decoded)&&decoded.right==-200);
 assert(!lease.receive(decoded,7,0,false)&&!lease.receive(decoded,8,0,true));
 assert(lease.receive(decoded,7,0,true));
 assert(lease.output(7,199,true,l,r)&&l==100&&r==-200);
 assert(!lease.output(7,200,true,l,r)&&l==0&&r==0);
 assert(!lease.output(7,1,false,l,r)&&!lease.neutral(7,1));
 s={7,0,0,true};assert(lease.receive(s,7,0xfffffff0U,true));assert(lease.neutral(7,5));
 assert(!lease.fresh(7,200));lease.reset();assert(!lease.fresh(7,5));
 s.left=1;assert(!missionStickEncode(s,p));
 std::cout<<"Mission stick lease passed\n";
}
