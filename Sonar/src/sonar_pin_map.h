#pragma once

// Candidate direct-wire map for the ESP32-DevKitC V4 and Open Echo Shield 002.
// Values are documentation/configuration only: the disarmed firmware does not
// configure these pins or communicate with the shield.
namespace sonar_pins {
constexpr int VOUT_ADC = 34; // ADC1_CH6; input-only GPIO
constexpr bool VOUT_ADC_AUDIT_SIGNED = false; // Measure signal range/conditioning before enabling.
constexpr int IO1 = 25;
constexpr int IO2 = 26;
constexpr int OUT3 = 27;
constexpr int OUT4 = 32;
constexpr int TUSS_CS = 21;
constexpr int SPI_SCK = 18;
constexpr int SPI_MISO = 19;
constexpr int SPI_MOSI = 23;
constexpr bool TUSS_SPI_AUDIT_SIGNED = false; // Keep SPI inactive until board/shield levels are checked.
constexpr int BOAT_UART_TX = 22;
}

static_assert(sonar_pins::VOUT_ADC == 34, "VOUT must remain on ADC1 input-only GPIO34");
static_assert(sonar_pins::TUSS_CS != sonar_pins::SPI_SCK &&
              sonar_pins::TUSS_CS != sonar_pins::SPI_MISO &&
              sonar_pins::TUSS_CS != sonar_pins::SPI_MOSI,
              "TUSS chip select must be separate from SPI pins");
