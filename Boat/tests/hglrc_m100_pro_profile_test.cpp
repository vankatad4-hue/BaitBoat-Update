#include <cassert>
#include "../source/hglrc_m100_pro_profile.h"
int main(){
 auto p=hglrcM100ProUnassigned();assert(!hglrcM100ProProfileValid(p));
 p={15,16,9600,true,true,true,true};assert(hglrcM100ProProfileValid(p));
 p.nmeaObserved=false;assert(!hglrcM100ProProfileValid(p));
 p.nmeaObserved=true;p.tx=15;assert(!hglrcM100ProProfileValid(p));
 p.tx=16;p.baud=38400;assert(!hglrcM100ProProfileValid(p));
}
