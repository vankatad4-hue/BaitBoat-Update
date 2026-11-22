/*
 * Flytec V080 R22 ADS1115 BATTERY BENCH — readout only, no actuation.
 *
 * This is NOT Boat mission firmware. No motor, solenoid, radio or navigation
 * libraries are linked. Uses one manually approved I2C pair only.
 *
 * Compiles DISABLED by default. Do not edit enable flags merely to make
 * output appear: first verify the exact purchased ESP32-S3 board pins
 * against its schematic, AND verify ADS1115 VDD/AIN0 power sequencing.
 *
 * To enable during physically isolated bench testing, copy
 * boat_adc_bench_local.example.h beside this sketch as
 * boat_adc_bench_local.h; fill the signed audit fields explicitly.
 * Do not commit locally approved pin values into the project baseline.
 */
#include <Arduino.h>
#include <Wire.h>
#include "boat_ads1115_bench_math.h"
#if __has_include("boat_adc_bench_local.h")
#include "boat_adc_bench_local.h"
#endif
#ifndef BOAT_ADC_BENCH_PIN_AUDIT_SIGNED
#define BOAT_ADC_BENCH_PIN_AUDIT_SIGNED 0
#endif
#ifndef BOAT_ADC_BENCH_SDA_GPIO
#define BOAT_ADC_BENCH_SDA_GPIO -1
#endif
#ifndef BOAT_ADC_BENCH_SCL_GPIO
#define BOAT_ADC_BENCH_SCL_GPIO -1
#endif

// R20 candidate GPIO8 SDA / GPIO7 SCL only. All other pins fail closed
// until documentation has been formally revised for the physical PCB.
#if (BOAT_ADC_BENCH_PIN_AUDIT_SIGNED == 1) && \
    (BOAT_ADC_BENCH_SDA_GPIO == 8) && (BOAT_ADC_BENCH_SCL_GPIO == 7)
#define BOAT_ADC_BENCH_READY_FOR_SENSOR_ONLY_I2C 1
#else
#define BOAT_ADC_BENCH_READY_FOR_SENSOR_ONLY_I2C 0
#endif

namespace {
constexpr uint8_t kAdsAddress=0x48; // ADDR physically strapped to sensor ground
constexpr uint8_t kConfigRegister=1,kConversionRegister=0;
static uint32_t lastReport=0;
bool readReg16(uint8_t reg,uint16_t &value) {
  Wire.beginTransmission(kAdsAddress);
  Wire.write(reg);
  if(Wire.endTransmission(false)!=0)return false;
  if(Wire.requestFrom(kAdsAddress,(uint8_t)2)!=(size_t)2)return false;
  const int high=Wire.read(),low=Wire.read();
  if(high<0||low<0)return false;
  value=(uint16_t)(((uint16_t)high<<8)|(uint16_t)low);
  return true;
}
bool triggerSingle() {
  Wire.beginTransmission(kAdsAddress);
  Wire.write(kConfigRegister);
  Wire.write((uint8_t)(BOAT_ADS1115_CONFIG_AIN0_SINGLE_128SPS_4V096>>8));
  Wire.write((uint8_t)BOAT_ADS1115_CONFIG_AIN0_SINGLE_128SPS_4V096);
  return Wire.endTransmission()==0;
}
void printError(uint32_t now,const char *error) {
  Serial.printf("%lu,ERROR,0,0,0,%s,UNCOMMISSIONED\n",
                (unsigned long)now,error);
}
} // namespace

void setup(){
  Serial.begin(115200);
  delay(250);
  Serial.println("# Flytec R22 ADS1115 diagnostic only. NO ACTUATOR/RADIO POWER.");
  Serial.println("# Never use estimated pack voltage as safety authorization.");
  Serial.println("# Check VDD-vs-AIN0 sequencing externally; this sketch cannot.");
  Serial.println("# ADS1015 and ADS1115 share address/registers: address ACK cannot prove chip identity.");
  Serial.println("ms,state,raw_code,ain_mV,estimated_pack_mV,reason,safety_authority");
#if BOAT_ADC_BENCH_READY_FOR_SENSOR_ONLY_I2C
  if(!Wire.begin(BOAT_ADC_BENCH_SDA_GPIO,BOAT_ADC_BENCH_SCL_GPIO,100000)){
    Serial.println("# I2C initialization failed. No readings.");
    return;
  }
  Wire.setTimeOut(15);
  Serial.println("# AUDIT FLAGS SET: read-only output enabled for sensor-only bench.");
  Serial.println("# WARNING: firmware cannot verify the physical audit or supply sequencing.");
#else
  Serial.println("# LOCKED: add signed local config only after physical pin and power audit.");
  Serial.println("# GPIO untouched. ADS1115 bus not initialized.");
#endif
}

void loop(){
#if BOAT_ADC_BENCH_READY_FOR_SENSOR_ONLY_I2C
  const uint32_t now=millis();
  if((uint32_t)(now-lastReport)<1000U)return;
  lastReport=now;
  if(!triggerSingle()){printError(now,"I2C_WRITE");return;}
  // At 128SPS nominal conversion takes 7.8125ms. Non-mission bench only.
  delay(12);
  uint16_t config=0;
  if(!readReg16(kConfigRegister,config)){printError(now,"I2C_CONFIG");return;}
  if((config&0x8000U)==0){printError(now,"CONVERSION_NOT_READY");return;}
  // Verify the requested AIN0 / single shot / 4.096V gain / 128SPS config;
  // do not silently convert with an unexpected ADC setting.
  if((config&0x7FFFU)!=
     (BOAT_ADS1115_CONFIG_AIN0_SINGLE_128SPS_4V096&0x7FFFU)){
    printError(now,"CONFIG_MISMATCH");return;
  }
  uint16_t rawRegister=0;
  if(!readReg16(kConversionRegister,rawRegister)){
    printError(now,"I2C_CONVERSION");return;
  }
  const auto r=boatAdcBenchDecode((int16_t)rawRegister);
  Serial.printf("%lu,%s,%d,%u,%u,%s,UNCOMMISSIONED\n",
                (unsigned long)now,
                r.plausible()?"READING":"REJECTED",
                (int)r.raw,
                (unsigned)r.inputMilliVolts,
                (unsigned)r.estimatedPackMilliVolts,
                boatAdcBenchReasonText(r.reason));
#else
  delay(1000);
#endif
}
