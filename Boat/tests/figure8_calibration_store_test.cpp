#include <cassert>
#include "../source/figure8_calibration_store.h"
static Figure8StoredCalibration slots[2]{};
static bool failWrites=false;
static bool read(uint8_t i,Figure8StoredCalibration &r){if(i>1)return false;r=slots[i];return true;}
static bool write(uint8_t i,const Figure8StoredCalibration &r){
 if(i>1||failWrites)return false;slots[i]=r;return true;
}
int main(){
 Figure8CalibrationStore s({read,write});
 Figure8StoredCalibration out{},candidate{};
 assert(!s.load(out));
 assert(s.save(candidate));assert(s.load(out)&&out.sequence==1);
 candidate.gyOffsetX=2.5f;
 assert(s.save(candidate));assert(s.load(out)&&out.sequence==2&&out.gyOffsetX==2.5f);
 slots[1].crc^=1;
 assert(s.load(out)&&out.sequence==1);
 failWrites=true;assert(!s.save(candidate));
 assert(s.load(out)&&out.sequence==1);
 failWrites=false;candidate.gyScaleX=0;
 assert(!s.save(candidate));assert(s.load(out)&&out.sequence==1);
}
