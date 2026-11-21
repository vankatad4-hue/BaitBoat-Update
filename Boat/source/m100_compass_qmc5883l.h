#pragma once
#include <stdint.h>
#include "carrier_sensor_math.h"
// HGLRC M100 PRO QMC5883L candidate (I2C 0x0D).
// Some production units use QMC5883P: do NOT treat P as L.
inline bool m100Qmc5883lIdentity(uint8_t id){return id==0xff;}
inline bool m100Qmc5883lDecode(const uint8_t data[7],SensorVector &out){
 if(data[6]&0x02u)return false; // overflow
 if(!(data[6]&0x01u))return false; // data not ready
 auto signed16=[](const uint8_t *p)->int16_t{
  return (int16_t)((uint16_t)p[0]|((uint16_t)p[1]<<8));
 };
 // QMC5883L 8G full scale: 3000 LSB/G = 30 LSB/uT.
 out={signed16(data)/30.0,signed16(data+2)/30.0,signed16(data+4)/30.0};
 return true;
}
