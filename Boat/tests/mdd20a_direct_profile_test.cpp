#include "../source/mdd20a_direct_profile.h"
#include <cassert>
#include <iostream>
int main(){
 // Synthetic test GPIOs only; this is not a supported Tracker pin map.
 const uint64_t allowed=(uint64_t(1)<<6)-1;
 auto p=mdd20aUnassignedProfile();
 assert(!mdd20aDirectProfileValid(p,allowed));
 p={true,0,1,2,3,4,5};
 assert(mdd20aDirectProfileValid(p,allowed));
 p.dirRight=2;assert(!mdd20aDirectProfileValid(p,allowed));
 p.dirRight=3;p.permit=6;assert(!mdd20aDirectProfileValid(p,allowed));
 p.permit=-1;assert(!mdd20aDirectProfileValid(p,allowed));
 p.permit=49;assert(!mdd20aDirectProfileValid(p,UINT64_MAX));
 p.permit=5;p.pinAuditSigned=false;assert(!mdd20aDirectProfileValid(p,allowed));
 std::cout<<"Direct MDD20A profile rejects unassigned, duplicate and reserved pins\n";
}
