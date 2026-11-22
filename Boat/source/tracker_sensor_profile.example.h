#pragma once
#include "tracker_sensor_port.h"
#include "boat_battery_divider.h"
// Copy locally to tracker_commissioned_config.h ONLY after documented audit.
// Contains R20 candidate GPIOs only; pinAuditSigned and all calibration flags remain false.
inline TrackerSensorConfig trackerLocalProfile(){
 TrackerSensorConfig c{};
 c.pinAuditSigned=false;
 c.gnssRx=5; c.gnssTx=4; c.sda=8; c.scl=7; // R20 candidates; not signed
 c.gnssBaud=115200; // HGLRC M100 PRO manufacturer default UBX NAV-PVT; verify physical output
 c.imuAddress=0x68;c.adcAddress=0x48; // GY-91 MPU9250 / ADS1115
 c.heading.commissioned=false; // QMC5883L is integrated on M100 PRO; verify I2C scan
 // R27: INA231 selected for BUS-only 3S voltage sense (0x40 candidate).
 // The ADC fields below remain a DISABLED LEGACY ADS1115 diagnostic fixture;
 // tracker_sensor_port.cpp blocks ADS polling after R27 selection. Do not
 // sign this profile or physically connect the old battery divider.
 c.battery={false,BOAT_BATTERY_DIVIDER_RATIO,1.0,0.0,9000,13000};
 return c;
}
