#include "tracker_sensor_port.h"
#include "ina231_voltage_contract.h"
#include "ina231_arduino_wire.h"
namespace{
 bool validPins(const TrackerSensorConfig &c){
  if(!c.pinAuditSigned||!(c.gnssBaud==9600||c.gnssBaud==115200)||(c.imuAddress!=0x68&&c.imuAddress!=0x69)||
     (boat_r27::INA231_MODULE_SELECTED ?
       c.ina231Address!=boat_r27::I2C_CANDIDATE_ADDRESS :
       (c.adcAddress<0x48||c.adcAddress>0x4b)))return false;
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
 delay(2);return writeReg(mag,0x0a,0x12); // continuous 8 Hz, 16-bit; 20 ms polling avoids overrun
}
bool TrackerSensorPort::begin(const TrackerSensorConfig &cfg){
 if(configured_||!validPins(cfg))return false;
 cfg_=cfg;
 // UART receive only in this bring-up; no undocumented GNSS setup command.
 serial_.begin(cfg.gnssBaud,SERIAL_8N1,cfg.gnssRx,cfg.gnssTx);
 if(!wire_.begin(cfg.sda,cfg.scl,100000))return false;
 wire_.setTimeOut(10);
 ina231Ready_=ina231Sampler_.begin(
  boat_r27::INA231_PHYSICAL_COMMISSIONED && cfg.pinAuditSigned &&
  cfg.ina231BoardAndWiringSigned && cfg.ina231DmmCalibrationSigned,
  cfg.ina231Address);
 imuReady_=initializeImu();bmpReady_=initializeBmp();m100CompassReady_=initializeM100Compass();configured_=true;
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
 rawAccel_=a;rawMag_=m;rawAt_=now;rawValid_=true;
 if(compassCal_.active())compassCal_.add(m);
 if(gy91Figure8_.active())gy91Figure8_.add(m);
 autoFigure8_.addGy(m,now);
 headingValid_=carrierHeading(a,m,cfg_.heading,heading_);
 if(headingValid_)headingAt_=now;
}
void TrackerSensorPort::pollAdc(uint32_t now){
 // R28 selected INA231: the real TwoWire transport is integrated but
 // cannot start while INA231_PHYSICAL_COMMISSIONED=false (R27/R28).
 // The legacy ADS1115 fallback is unreachable while this module is selected.
 if(boat_r27::INA231_MODULE_SELECTED){
  if(!ina231Ready_){adcPending_=false;voltageValid_=false;return;}
  Ina231ArduinoWireR28 bus(wire_);
  const auto status=ina231Sampler_.poll(bus,now);
  const auto fresh=ina231Sampler_.snapshot(now);
  voltageValid_=status!=boat_r28::PollResult::Failed && fresh.valid;
  voltage_=voltageValid_?fresh.millivolts:0;
  voltageAt_=fresh.acquiredAtMs;
  return;
 }
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
 for(unsigned i=0;i<256&&serial_.available();++i){const int c=serial_.read();if(c>=0){if(cfg_.gnssBaud==115200)m100Ubx_.feed((uint8_t)c,now);else gnss_.feed((char)c,now);}}
 pollImu(now);pollAdc(now);pollBmp(now);pollM100Compass(now);
 autoFigure8_.update(now);
}
void TrackerSensorPort::copySensors(BoatHardwareSample &s,uint32_t now)const{
 if(cfg_.gnssBaud==115200){
  const auto g=m100Ubx_.snapshot(now);
  s.fix.latE7=g.latE7;s.fix.lonE7=g.lonE7;s.fix.accuracyM=g.accuracyM;
  s.fix.fresh=g.positionValid;s.gnssAt=g.at;s.gpsFixType=g.positionValid?g.fixType:0;
  s.satellites=g.satellites;s.speedValid=g.speedValid;
  s.speedCmPerSecond=g.speedCmS;s.speedAt=g.at;
 }else{
 auto g=gnss_.snapshot(now);
 s.fix.latE7=g.latE7;s.fix.lonE7=g.lonE7;s.fix.accuracyM=g.estimatedAccuracyM;s.fix.fresh=g.positionValid;s.gnssAt=g.at;
 s.gpsFixType=g.fixType;s.satellites=g.satellites;s.speedValid=g.speedValid;s.speedCmPerSecond=g.speedCmS;s.speedAt=g.at;
 }
 s.fix.headingDeg=heading_;s.headingAt=headingAt_;s.fix.headingValid=headingValid_&&now-headingAt_<300U;
 s.batteryMillivolts=voltage_;s.batteryAt=voltageAt_;s.voltageValid=voltageValid_&&now-voltageAt_<1000U;
 // No guessed state-of-charge or battery/geofence safety authorization.
 s.batteryPercent=255;
}

bool TrackerSensorPort::initializeBmp(){
 uint8_t id=0,cal[24]={};
 for(uint8_t address: {0x76,0x77}){
  if(!readRegs(address,0xD0,&id,1)||id!=0x58)continue;
  if(!readRegs(address,0x88,cal,24)||!bmp280DecodeCalibration(cal,bmpCal_))continue;
  // Config: standby 1000ms, IIR filter x4; ctrl_meas normal mode, x1 temp/x1 pressure.
  if(!writeReg(address,0xF5,0xA8)||!writeReg(address,0xF4,0x27))continue;
  bmpAddress_=address;return true;
 }
 return false;
}
void TrackerSensorPort::pollBmp(uint32_t now){
 if(!bmpReady_||(uint32_t)(now-bmpPollAt_)<1000U)return;
 bmpPollAt_=now;uint8_t b[6]={};
 if(!readRegs(bmpAddress_,0xF7,b,6)){bmpValid_=false;return;}
 const uint32_t p=((uint32_t)b[0]<<12)|((uint32_t)b[1]<<4)|(b[2]>>4);
 const uint32_t t=((uint32_t)b[3]<<12)|((uint32_t)b[4]<<4)|(b[5]>>4);
 bmpValid_=bmp280Compensate(bmpCal_,t,p,bmpTemp_,bmpPressure_);
 if(bmpValid_)bmpAt_=now;
}

bool TrackerSensorPort::initializeM100Compass(){
 uint8_t id=0;
 // QMC5883L identity is 0xFF at register 0x0D. No blind fallback for
 // QMC5883P replacement silicon: its register map differs.
 if(!readRegs(0x0d,0x0d,&id,1)||!m100Qmc5883lIdentity(id))return false;
 return writeReg(0x0d,0x0b,0x01)&&writeReg(0x0d,0x09,0x1d);
}
void TrackerSensorPort::pollM100Compass(uint32_t now){
 if(!m100CompassReady_||(uint32_t)(now-m100CompassPollAt_)<100U)return;
 m100CompassPollAt_=now;
 uint8_t data[7]={};SensorVector m{};
 if(readRegs(0x0d,0x00,data,7)&&m100Qmc5883lDecode(data,m)){
  if(m100Figure8_.active())m100Figure8_.add(m);
  autoFigure8_.addM100(m,now);
 }
}
