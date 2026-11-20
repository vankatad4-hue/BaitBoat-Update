#pragma once
#include <Arduino.h>

// ADS1115 four-axis joystick reader. Address 0x4A is a provisional strap;
// confirm ADDR pin wiring on the carrier before hardware use.
struct JoystickAdcConfig {
  uint8_t address;
  uint8_t channelLeftX;
  uint8_t channelLeftY;
  uint8_t channelRightX;
  uint8_t channelRightY;
};
bool joystickAdc_begin(const JoystickAdcConfig &config = {0x4A,0,1,2,3});
bool joystickAdc_read(int &leftX, int &leftY, int &rightX, int &rightY);
bool joystickAdc_isReady();
