#pragma once
#include "tracker_sensor_port.h"
// Copy locally to tracker_commissioned_config.h ONLY after documented audit.
// Contains R20 candidate GPIOs only; pinAuditSigned and all calibration flags remain false.
inline TrackerSensorConfig trackerLocalProfile(){
 TrackerSensorConfig c{};
 c.pinAuditSigned=false;
 c.gnssRx=5; c.gnssTx=4; c.sda=8; c.scl=7; // R20 candidates; not signed
 c.gnssBaud=115200; // HGLRC M100 PRO manufacturer default UBX NAV-PVT; verify physical output
 c.imuAddress=0x68;c.adcAddress=0x48; // GY-91 MPU9250 / ADS1115
 c.heading.commissioned=false; // QMC5883L is integrated on M100 PRO; verify I2C scan
 // ADS1115 AIN0 divider candidate: 100k BAT+ to AIN0, 33k AIN0 to GND
 // (ratio 4.0303), 100nF AIN0-to-GND. Calibrate against a DMM before enabling.
 c.battery={false,4.0303030303,1.0,0.0,9000,12600};
 return c;
}
