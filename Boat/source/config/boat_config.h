/*
  Flytec V080 boat configuration — intentionally unassigned.

  Selected controller: Heltec Wireless Tracker V1.2 (ESP32-S3FN8 + UC6580 GNSS).
  Selected motion/heading sensor: GY-91-style MPU-9250 + BMP280.
  Primary command radio remains external nRF24L01+ PA/LNA.
  Backup is Tracker onboard SX1262 over SPI, not external UART E22.
  Backup unavailable until Remote E22 RF compatibility proven.

  Do not assign pins until the exact V1.2 schematic/pin map, GY-91 breakout
  revision and all peripheral conflicts have been checked.
*/

#pragma once

// TODO: Define verified hardware pin assignments after V1.2 pinout review.
// No sensor, radio or actuator pins are assigned in this scaffold.

// Active architecture: Heltec is the sole Boat processor. Cytron MDD20A
// uses direct PWM/DIR; independent electrical interlocks and passive I/O
// expansion are permitted. No secondary programmable controller or protocol.
