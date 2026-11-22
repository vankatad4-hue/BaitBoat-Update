#pragma once
#include "tracker_sensor_port.h"
// Copy locally to tracker_commissioned_config.h ONLY after documented audit.
// This example intentionally has no valid GPIO mapping or calibration.
inline TrackerSensorConfig trackerLocalProfile(){
 TrackerSensorConfig c{};
 c.pinAuditSigned=false;
 c.gnssRx=c.gnssTx=c.sda=c.scl=-1;
 c.gnssBaud=115200; // HGLRC M100 PRO manufacturer default UBX NAV-PVT; verify physical output
 c.imuAddress=0x68;c.adcAddress=0x48;
 c.heading.commissioned=false;
 // ADS1115 AIN0 divider candidate: 100k BAT+ to AIN0, 33k AIN0 to GND
 // (ratio 4.0303), 100nF AIN0-to-GND. Calibrate against a DMM before enabling.
 c.battery={false,4.0303030303,1.0,0.0,9000,12600};
 return c;
}
