#pragma once
#include <stdint.h>
#include <math.h>
// Bosch BMP280 integer compensation, calibrated coefficients from 0x88..0x9F.
struct Bmp280Calibration {
 uint16_t t1,p1; int16_t t2,t3,p2,p3,p4,p5,p6,p7,p8,p9;
};
inline uint16_t bmpU16(const uint8_t *p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
inline int16_t bmpI16(const uint8_t *p){return (int16_t)bmpU16(p);}
inline bool bmp280DecodeCalibration(const uint8_t bytes[24],Bmp280Calibration &c){
 c={bmpU16(bytes),bmpU16(bytes+6),bmpI16(bytes+2),bmpI16(bytes+4),
    bmpI16(bytes+8),bmpI16(bytes+10),bmpI16(bytes+12),bmpI16(bytes+14),
    bmpI16(bytes+16),bmpI16(bytes+18),bmpI16(bytes+20),bmpI16(bytes+22)};
 return c.t1!=0&&c.t1!=0xffff&&c.p1!=0&&c.p1!=0xffff;
}
inline bool bmp280Compensate(const Bmp280Calibration &c,uint32_t adcT,uint32_t adcP,
                            int32_t &temperatureCentiC,uint32_t &pressurePa){
 if(adcT==0x80000||adcP==0x80000||adcT>0xfffff||adcP>0xfffff||!c.p1)return false;
 int32_t v1=((int32_t)(adcT>>3)-((int32_t)c.t1<<1))*c.t2>>11;
 int32_t d=((int32_t)(adcT>>4)-(int32_t)c.t1);
 int32_t v2=(int32_t)(((((int64_t)d*d)>>12)*(int64_t)c.t3)>>14);
 int32_t fine=v1+v2;temperatureCentiC=(fine*5+128)>>8;
 int64_t x=(int64_t)fine-128000;
 int64_t y=x*x*(int64_t)c.p6;
 y+=((x*(int64_t)c.p5)<<17);
 y+=((int64_t)c.p4<<35);
 x=((x*x*(int64_t)c.p3)>>8)+((x*(int64_t)c.p2)<<12);
 x=((((int64_t)1<<47)+x)*(int64_t)c.p1)>>33;
 if(!x)return false;
 int64_t p=1048576-(int64_t)adcP;
 p=(((p<<31)-y)*3125)/x;
 x=((int64_t)c.p9*(p>>13)*(p>>13))>>25;
 y=((int64_t)c.p8*p)>>19;
 p=((p+x+y)>>8)+((int64_t)c.p7<<4);
 if(p<0)return false;
 pressurePa=(uint32_t)((p+128)>>8);
 return pressurePa>=30000&&pressurePa<=120000&&temperatureCentiC>=-4000&&temperatureCentiC<=8500;
}
