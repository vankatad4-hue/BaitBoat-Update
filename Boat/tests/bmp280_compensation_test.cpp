#include <cassert>
#include <cstdint>
#include "../source/bmp280_compensation.h"
int main(){
 // Bosch BMP280 datasheet example calibration and raw readings.
 Bmp280Calibration c{27504,26435,-1000,36477,-10685,3024,2855,140,-7,15500,-14600,6000};
 int32_t t=0;uint32_t p=0;
 assert(bmp280Compensate(c,519888,415148,t,p));
 assert(t>=2507&&t<=2510);
 assert(p>=100640&&p<=100670);
 assert(!bmp280Compensate(c,0x80000,415148,t,p));
 assert(!bmp280Compensate(c,519888,0x80000,t,p));
 c.p1=0;assert(!bmp280Compensate(c,519888,415148,t,p));
}
