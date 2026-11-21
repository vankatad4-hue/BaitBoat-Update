#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "boat_sd_module_profile.h"
// Diagnostic SD module only. Never drives motors/solenoids. Caller must
// verify shared SPI bus arbitration with the nRF24 radio before deployment.
class BoatSdModuleArduino {
public:
 bool begin(const BoatSdModuleProfile &p,SPIClass &spi){
  if(ready_||!boatSdModuleProfileValid(p))return false;
  pinMode(p.cs,OUTPUT);digitalWrite(p.cs,HIGH);
  pinMode(p.nrfCs,OUTPUT);digitalWrite(p.nrfCs,HIGH);
  spi.begin(p.sck,p.miso,p.mosi,p.cs);
  if(!SD.begin(p.cs,spi,p.maxClockHz)){SD.end();return false;}
  ready_=true;return true;
 }
 bool ready()const{return ready_;}
 bool read(const char *path,uint8_t *buffer,size_t len){
  if(!ready_||!path||!buffer||!len)return false;
  File f=SD.open(path,FILE_READ);
  if(!f||f.size()!=(uint32_t)len){if(f)f.close();return false;}
  const bool ok=f.read(buffer,len)==len;f.close();return ok;
 }
 // Uses a temporary file then renames it. FAT rename is not guaranteed
 // power-loss atomic; retain two CRC/sequence validated independent slots.
 bool write(const char *path,const uint8_t *buffer,size_t len){
  if(!ready_||!path||!buffer||!len)return false;
  SD.remove(path);
  File f=SD.open(path,FILE_WRITE);
  if(!f)return false;
  const bool ok=f.write(buffer,len)==len&&f.flush();
  f.close();if(!ok){SD.remove(path);return false;}
  return true;
 }
 void end(){if(ready_){SD.end();ready_=false;}}
private:
 bool ready_=false;
};
