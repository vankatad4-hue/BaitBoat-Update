#include "tracker_sensor_port.h"
namespace{
 bool validPins(const TrackerSensorConfig &c){
  if(!c.pinAuditSigned||!(c.gnssBaud==9600||c.gnssBaud==115200)||(c.imuAddress!=0x68&&c.imuAddress!=0x69)||c.adcAddress<0x48||c.adcAddress>0x4b)return false;
  int pins[]={c.gnssRx,c.gnssTx,c.sda,c.scl};
  for(unsigned i=0;i<4;++i){if(pins[i]<0||pins[i]>48)return false;for(unsigned j=0;j<i;++j)if(pins[i]==pins[j])return false;}
  return true;
 }
 int16_t big(const uint8_t *p){return (int16_t)(((uint16_t)p[0]<<8)|p[1]);}
 int16_t little(const uint8_t *p){return (int16_t)(((uint16_t)p[1]<<8)|p[0]);}
}
bool TrackerSensorPort::writeReg(uint8_t address,uint8_t reg,uint8_t value){
 wire_.beginTransmission(address);wire_.write(reg);wire_.write(value);return wire_.endTransmission()==0;
}
bool TrackerSensorPort::readRegs(uint8_t address,uint8_t reg,uint8_t *dst,uint8_t length){
 wire_.beginTransmission(address);wire_.write(reg);
 if(wire_.endTransmission(false)!=0||wire_.requestFrom(address,length)!=(size_t)length)return false;
 for(uint8_t i=0;i<length;++i){const int v=wire_.read();if(v<0)return false;dst[i]=(uint8_t)v;}return true;
}
bool TrackerSensorPort::writeAdc(uint16_t value){
 wire_.beginTransmission(cfg_.adcAddress);wire_.write((uint8_t)1);wire_.write((uint8_t)(value>>8));wire_.write((uint8_t)value);return wire_.endTransmission()==0;
}
bool TrackerSensorPort::initializeImu(){
 uint8_t v=0;const uint8_t a=cfg_.imuAddress,mag=0x0c;
 if(!readRegs(a,0x75,&v,1)||v!=0x71)return false;
 if(!writeReg(a,0x6b,0x01))return false;
 delay(100); // setup only, before any actuation is possible
 if(!writeReg(a,0x1c,0x00)||!writeReg(a,0x6a,0x00)||!writeReg(a,0x37,0x02))return false;
 if(!readRegs(mag,0x00,&v,1)||v!=0x48||!writeReg(mag,0x0a,0))return false;
 delay(2);
 if(!writeReg(mag,0x0a,0x0f))return false;
 delay(2);uint8_t asa[3];if(!readRegs(mag,0x10,asa,3))return false;
 for(unsigned i=0;i<3;++i){if(!asa[i]||asa[i]==255)return false;adjust_[i]=(asa[i]-128.0)/256.0+1.0;}
 if(!writeReg(mag,0x0a,0))return false;
 delay(2);return writeReg(mag,0x0a,0x16); // continuous 100 Hz, 16-bit
}
bool TrackerSensorPort::begin(const TrackerSensorConfig &cfg){
 if(configured_||!validPins(cfg))return false;
 cfg_=cfg;
 // UART receive only in this bring-up; no undocumented GNSS setup command.
 serial_.begin(cfg.gnssBaud,SERIAL_8N1,cfg.gnssRx,cfg.gnssTx);
 if(!wire_.begin(cfg.sda,cfg.scl,100000))return false;
 wire_.setTimeOut(10);
 imuReady_=initializeImu();configured_=true;
 return true; // GNSS and ADC diagnostics remain possible if IMU not identified
}
void TrackerSensorPort::pollImu(uint32_t now){
 if(!imuReady_||now-imuPollAt_<20U)return;
 imuPollAt_=now;uint8_t st=0,mag[7],accel[6];
 if(!readRegs(0x0c,0x02,&st,1)){headingValid_=false;return;}
 if(!(st&1U))return;
 if(!readRegs(0x0c,0x03,mag,7)||!readRegs(cfg_.imuAddress,0x3b,accel,6)||(mag[6]&8U)||(st&2U)){headingValid_=false;return;}
 SensorVector a={big(accel)/16384.0,big(accel+2)/16384.0,big(accel+4)/16384.0};
 SensorVector m={little(mag)*0.15*adjust_[0],little(mag+2)*0.15*adjust_[1],little(mag+4)*0.15*adjust_[2]};
 headingValid_=carrierHeading(a,m,cfg_.heading,heading_);
 if(headingValid_)headingAt_=now;
}
void TrackerSensorPort::pollAdc(uint32_t now){
 if(!adcPending_){
  adcAt_=now;adcPending_=writeAdc(0xc383);if(!adcPending_)voltageValid_=false;return;
 }
 if(now-adcAt_<10U)return;
 uint8_t v[2];if(!readRegs(cfg_.adcAddress,1,v,2)){adcPending_=voltageValid_=false;return;}
 if(!(v[0]&0x80U)){if(now-adcAt_>=30U)adcPending_=voltageValid_=false;return;}
 adcPending_=false;
 voltageValid_=readRegs(cfg_.adcAddress,0,v,2)&&carrierBatteryMv(big(v),cfg_.battery,voltage_);
 if(voltageValid_)voltageAt_=now;
}
void TrackerSensorPort::poll(uint32_t now){
 if(!configured_)return;
 // Bounded RX work; at 115200 baud service frequently enough to prevent backlog.
 for(unsigned i=0;i<256&&serial_.available();++i){const int c=serial_.read();if(c>=0)gnss_.feed((char)c,now);}
 pollImu(now);pollAdc(now);
}
void TrackerSensorPort::copySensors(BoatHardwareSample &s,uint32_t now)const{
 auto g=gnss_.snapshot(now);
 s.fix.latE7=g.latE7;s.fix.lonE7=g.lonE7;s.fix.accuracyM=g.estimatedAccuracyM;s.fix.fresh=g.positionValid;s.gnssAt=g.at;
 s.gpsFixType=g.fixType;s.satellites=g.satellites;s.speedValid=g.speedValid;s.speedCmPerSecond=g.speedCmS;s.speedAt=g.at;
 s.fix.headingDeg=heading_;s.headingAt=headingAt_;s.fix.headingValid=headingValid_&&now-headingAt_<300U;
 s.batteryMillivolts=voltage_;s.batteryAt=voltageAt_;s.voltageValid=voltageValid_&&now-voltageAt_<1000U;
 // No guessed state-of-charge or battery/geofence safety authorization.
 s.batteryPercent=255;
}
