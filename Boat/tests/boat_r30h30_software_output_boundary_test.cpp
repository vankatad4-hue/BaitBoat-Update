#include "boat_r30h30_software_output_boundary.h"
#include <cassert>
#include <cstdint>
int main(){
 using boat_r30h30::validOutput;
 assert(validOutput(0,0,-1,100));
 assert(validOutput(-1000,1000,-1,1));
 for(int ch=0;ch<4;++ch)assert(validOutput(0,0,(int8_t)ch,100));
 assert(!validOutput(0,0,-1,0));
 assert(!validOutput(0,0,-1,101));
 assert(!validOutput(-1001,0,-1,100));
 assert(!validOutput(0,1001,-1,100));
 assert(!validOutput(0,0,-2,100));
 assert(!validOutput(0,0,4,100));
 assert(!validOutput(1,0,0,100));
 assert(!validOutput(0,-1,3,100));
 return 0;
}
