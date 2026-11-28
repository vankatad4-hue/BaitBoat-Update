#pragma once

#include <Arduino.h>
#include "boat_radio_hw_config.h"
#include "tracker_sensor_port.h"

// Optional UART0-to-DBT adapter. The compile profile enables code generation,
// but the port cannot start until its physical pin audit is explicitly signed.
class BoatOpenEchoUartPort {
public:
  explicit BoatOpenEchoUartPort(TrackerSensorPort &sensors) : sensors_(sensors) {}

  bool begin(bool sensorOwnerReady);
  void poll(uint32_t now);
  bool active() const { return active_; }

private:
  TrackerSensorPort &sensors_;
  bool active_ = false;
};
