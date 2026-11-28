#pragma once

#include <Arduino.h>
#include "boat_radio_hw_config.h"
#include "tracker_sensor_port.h"
#include "sonar_power_protocol.h"
#include "sonar_data_protocol.h"

// Optional UART0-to-DBT adapter. The compile profile enables code generation,
// but the port cannot start until its physical pin audit is explicitly signed.
class BoatOpenEchoUartPort {
public:
  explicit BoatOpenEchoUartPort(TrackerSensorPort &sensors) : sensors_(sensors) {}

  bool begin(bool sensorOwnerReady);
  void poll(uint32_t now);
  bool requestPower(uint16_t commandId, bool enabled, uint32_t now);
  bool takePowerResult(uint16_t &commandId, bool &success);
  bool active() const { return active_; }

private:
  TrackerSensorPort &sensors_;
  bool active_ = false;
  bool powerPending_ = false;
  bool powerFrameSent_ = false;
  bool powerResultReady_ = false;
  bool requestedEnabled_ = false;
  bool powerResultSuccess_ = false;
  uint16_t powerCommandId_ = 0U, powerResultId_ = 0U;
  uint32_t powerSendAtMs_ = 0U, powerDeadlineMs_ = 0U;
  uint8_t powerFrame_[sonar_power_v1::FRAME_SIZE]{};
  sonar_power_v1::Parser powerParser_{};
  sonar_data_v1::Parser dataParser_{};
};
