#pragma once
#include "tracker_sensor_port.h"
// Copy locally to tracker_commissioned_config.h ONLY after documented audit.
// This example intentionally has no valid GPIO mapping or calibration.
inline TrackerSensorConfig trackerLocalProfile(){
 TrackerSensorConfig c{};
 c.pinAuditSigned=false;
 c.gnssRx=c.gnssTx=c.sda=c.scl=-1;
 c.gnssBaud=9600; // verify actual receiver output before changing
 c.imuAddress=0x68;c.adcAddress=0x48;
 c.heading.commissioned=false;c.battery.commissioned=false;
 return c;
}
