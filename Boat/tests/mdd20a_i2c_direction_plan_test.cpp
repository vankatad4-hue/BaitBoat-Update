#include <cassert>
#include "../source/mdd20a_i2c_direction_plan.h"
int main() {
 auto x=mdd20aI2cDirectionPrepare(false,true,true,true,true,true,true);
 assert(!x.valid && x.bits==0);
 x=mdd20aI2cDirectionPrepare(true,true,true,true,true,true,false);
 assert(x.valid && x.bits==1);
 assert(!mdd20aI2cDirectionCommitVerified(x,true,false,1,true,true));
 assert(!mdd20aI2cDirectionCommitVerified(x,true,true,0,true,true));
 assert(!mdd20aI2cDirectionCommitVerified(x,true,true,1,false,true));
 assert(!mdd20aI2cDirectionCommitVerified(x,true,true,1,true,false));
 assert(mdd20aI2cDirectionCommitVerified(x,true,true,1,true,true));
 assert(!mdd20aI2cDirectionPrepare(true,true,false,true,true,false,false).valid);
 assert(!mdd20aI2cDirectionPrepare(true,true,true,false,true,false,false).valid);
 assert(!mdd20aI2cDirectionPrepare(true,true,true,true,false,false,false).valid);
 return 0;
}
