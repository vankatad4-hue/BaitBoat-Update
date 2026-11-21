#include <cassert>
#include "../source/figure8_calibration_record.h"
int main(){
 Figure8StoredCalibration a{},b{},selected{};
 assert(!figure8CalibrationValid(a));
 a.sequence=1;a.crc=figure8RecordCrc(a);
 assert(figure8CalibrationValid(a));
 b=a;b.sequence=2;b.crc=figure8RecordCrc(b);
 assert(figure8SelectRecord(a,b,selected)&&selected.sequence==2);
 b.gyScaleX=0;
 assert(figure8SelectRecord(a,b,selected)&&selected.sequence==1);
 a.crc^=1;
 assert(!figure8SelectRecord(a,b,selected));
}
