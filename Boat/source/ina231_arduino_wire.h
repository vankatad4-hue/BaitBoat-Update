#pragma once
#include <Wire.h>
#include "ina231_acquisition.h"
// R28 ESP32-S3 TwoWire adapter. Performs no I2C activity in constructor.
// Calling code must establish signed physical power/pinout/level validation
// AND explicit INA231_PHYSICAL_COMMISSIONED before allowing polling.
class Ina231ArduinoWireR28 final : public boat_r28::RegisterBus {
public:
 explicit Ina231ArduinoWireR28(TwoWire &wire):wire_(wire){}
 bool write16(uint8_t addr,uint8_t reg,uint16_t value) override {
  wire_.beginTransmission(addr);
  wire_.write(reg);
  wire_.write(static_cast<uint8_t>(value>>8));
  wire_.write(static_cast<uint8_t>(value));
  return wire_.endTransmission()==0;
 }
 bool read16(uint8_t addr,uint8_t reg,uint16_t &value) override {
  value=0;
  wire_.beginTransmission(addr);
  wire_.write(reg);
  if(wire_.endTransmission(false)!=0 ||
     wire_.requestFrom(addr,static_cast<uint8_t>(2))!=static_cast<size_t>(2))return false;
  const int hi=wire_.read(),lo=wire_.read();
  if(hi<0 || lo<0)return false;
  value=static_cast<uint16_t>((static_cast<uint16_t>(hi)<<8) | static_cast<uint8_t>(lo));
  return true;
 }
private:
 TwoWire &wire_;
};
