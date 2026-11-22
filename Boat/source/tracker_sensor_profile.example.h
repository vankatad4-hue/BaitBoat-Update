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
 // R21 unsigned candidate: 150k from switched BAT+ to AIN0, 33k to GND,
 // 100nF AIN0-to-GND. Max 12.6V => 2.272V on AIN0 (3.3V ADC).
 // Never back-power the unpowered ADC; use switched feed and verify rail
 // sequencing, high-voltage transients and DMM calibration before enabling.
 c.battery={false,BOAT_BATTERY_DIVIDER_RATIO,1.0,0.0,9000,13000};
 return c;
}
