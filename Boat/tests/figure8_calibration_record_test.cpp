#include <cassert>
#include "../source/figure8_calibration_record.h"
int main(){
 Figure8StoredCalibration golden{};
 golden.gyOffsetX=1.25f;golden.gyOffsetY=-2.5f;
 golden.gyScaleX=1.5f;golden.gyScaleY=0.75f;
 golden.m100OffsetX=0.125f;golden.m100OffsetY=-0.25f;
 golden.m100ScaleX=2.0f;golden.m100ScaleY=0.5f;
 golden.sequence=17;
 // Golden CRC from 48 LE bytes of <IHHffffffffII> (zlib CRC32).
 assert(figure8RecordCrc(golden)==0xFEAC9650U);
 golden.crc=0xDEADBEEFU;
 assert(figure8RecordCrc(golden)==0xFEAC9650U);
 golden.crc=figure8RecordCrc(golden);
 assert(figure8CalibrationValid(golden));

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
