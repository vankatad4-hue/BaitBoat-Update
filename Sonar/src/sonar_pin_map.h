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
constexpr bool BOAT_UART_TX_AUDIT_SIGNED = false; // Measure coprocessor TX before wiring Boat RX.
constexpr int BOAT_UART_RX = 17; // Candidate Boat-to-Sonar command return.
constexpr bool BOAT_UART_RX_AUDIT_SIGNED = false; // UART wake path remains electrically unsigned.

// These helpers make the provisional allocation fail at compile time if a
// future pin edit introduces an ESP32 classic input-only or boot-strapping pin
// on a signal that the firmware must drive.
constexpr bool isClassicEsp32Gpio(int gpio) { return gpio >= 0 && gpio <= 39; }
constexpr bool isInputOnlyClassicEsp32Gpio(int gpio) { return gpio >= 34 && gpio <= 39; }
constexpr bool isClassicEsp32FlashPin(int gpio) { return gpio >= 6 && gpio <= 11; }
constexpr bool isClassicEsp32StrapPin(int gpio) {
  return gpio == 0 || gpio == 2 || gpio == 5 || gpio == 12 || gpio == 15;
}
}

static_assert(sonar_pins::VOUT_ADC == 34, "VOUT must remain on ADC1 input-only GPIO34");
static_assert(sonar_pins::isClassicEsp32Gpio(sonar_pins::VOUT_ADC) &&
              sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::VOUT_ADC),
              "VOUT must use a classic ESP32 ADC1 input-only GPIO");
static_assert(sonar_pins::isClassicEsp32Gpio(sonar_pins::IO1) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::IO2) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::OUT3) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::OUT4) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::TUSS_CS) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::SPI_SCK) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::SPI_MISO) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::SPI_MOSI) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::BOAT_UART_TX) &&
              sonar_pins::isClassicEsp32Gpio(sonar_pins::BOAT_UART_RX),
              "Every provisional sonar signal must use a bonded ESP32 DevKitC GPIO");
static_assert(!sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::IO1) &&
              !sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::IO2) &&
              !sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::TUSS_CS) &&
              !sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::SPI_SCK) &&
              !sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::SPI_MOSI) &&
              !sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::BOAT_UART_TX) &&
              !sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::BOAT_UART_RX),
              "Driven sonar signals cannot use input-only GPIO34..39");
static_assert(!sonar_pins::isClassicEsp32StrapPin(sonar_pins::IO1) &&
              !sonar_pins::isClassicEsp32StrapPin(sonar_pins::IO2) &&
              !sonar_pins::isClassicEsp32StrapPin(sonar_pins::OUT3) &&
              !sonar_pins::isClassicEsp32StrapPin(sonar_pins::OUT4) &&
              !sonar_pins::isClassicEsp32StrapPin(sonar_pins::TUSS_CS) &&
              !sonar_pins::isClassicEsp32StrapPin(sonar_pins::SPI_SCK) &&
              !sonar_pins::isClassicEsp32StrapPin(sonar_pins::SPI_MISO) &&
              !sonar_pins::isClassicEsp32StrapPin(sonar_pins::SPI_MOSI) &&
              !sonar_pins::isClassicEsp32StrapPin(sonar_pins::BOAT_UART_TX),
              "Avoid classic ESP32 boot-strapping pins in the sonar allocation");
static_assert(!sonar_pins::isClassicEsp32FlashPin(sonar_pins::IO1) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::IO2) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::OUT3) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::OUT4) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::TUSS_CS) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::SPI_SCK) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::SPI_MISO) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::SPI_MOSI) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::BOAT_UART_TX),
              "Avoid classic ESP32 module flash GPIO6..11");
static_assert(sonar_pins::TUSS_CS != sonar_pins::SPI_SCK &&
              sonar_pins::TUSS_CS != sonar_pins::SPI_MISO &&
              sonar_pins::TUSS_CS != sonar_pins::SPI_MOSI,
              "TUSS chip select must be separate from SPI pins");
static_assert(sonar_pins::IO1 != sonar_pins::IO2 &&
              sonar_pins::IO1 != sonar_pins::OUT3 &&
              sonar_pins::IO1 != sonar_pins::OUT4 &&
              sonar_pins::IO1 != sonar_pins::TUSS_CS &&
              sonar_pins::IO1 != sonar_pins::SPI_SCK &&
              sonar_pins::IO1 != sonar_pins::SPI_MISO &&
              sonar_pins::IO1 != sonar_pins::SPI_MOSI &&
              sonar_pins::IO1 != sonar_pins::BOAT_UART_TX &&
              sonar_pins::IO2 != sonar_pins::OUT3 &&
              sonar_pins::IO2 != sonar_pins::OUT4 &&
              sonar_pins::IO2 != sonar_pins::TUSS_CS &&
              sonar_pins::IO2 != sonar_pins::SPI_SCK &&
              sonar_pins::IO2 != sonar_pins::SPI_MISO &&
              sonar_pins::IO2 != sonar_pins::SPI_MOSI &&
              sonar_pins::IO2 != sonar_pins::BOAT_UART_TX &&
              sonar_pins::OUT3 != sonar_pins::OUT4 &&
              sonar_pins::OUT3 != sonar_pins::TUSS_CS &&
              sonar_pins::OUT3 != sonar_pins::SPI_SCK &&
              sonar_pins::OUT3 != sonar_pins::SPI_MISO &&
              sonar_pins::OUT3 != sonar_pins::SPI_MOSI &&
              sonar_pins::OUT3 != sonar_pins::BOAT_UART_TX &&
              sonar_pins::OUT4 != sonar_pins::TUSS_CS &&
              sonar_pins::OUT4 != sonar_pins::SPI_SCK &&
              sonar_pins::OUT4 != sonar_pins::SPI_MISO &&
              sonar_pins::OUT4 != sonar_pins::SPI_MOSI &&
              sonar_pins::OUT4 != sonar_pins::BOAT_UART_TX &&
              sonar_pins::VOUT_ADC != sonar_pins::IO1 &&
              sonar_pins::VOUT_ADC != sonar_pins::IO2 &&
              sonar_pins::VOUT_ADC != sonar_pins::OUT3 &&
              sonar_pins::VOUT_ADC != sonar_pins::OUT4 &&
              sonar_pins::VOUT_ADC != sonar_pins::TUSS_CS &&
              sonar_pins::VOUT_ADC != sonar_pins::SPI_SCK &&
              sonar_pins::VOUT_ADC != sonar_pins::SPI_MISO &&
              sonar_pins::VOUT_ADC != sonar_pins::SPI_MOSI &&
              sonar_pins::VOUT_ADC != sonar_pins::BOAT_UART_TX &&
              sonar_pins::VOUT_ADC != sonar_pins::BOAT_UART_RX &&
              sonar_pins::BOAT_UART_TX != sonar_pins::TUSS_CS &&
              sonar_pins::SPI_SCK != sonar_pins::SPI_MISO &&
              sonar_pins::SPI_SCK != sonar_pins::SPI_MOSI &&
              sonar_pins::SPI_MISO != sonar_pins::SPI_MOSI &&
              sonar_pins::BOAT_UART_TX != sonar_pins::SPI_SCK &&
              sonar_pins::BOAT_UART_TX != sonar_pins::SPI_MISO &&
              sonar_pins::BOAT_UART_TX != sonar_pins::SPI_MOSI,
              "Each sonar signal must own a distinct ESP32 DevKitC GPIO");
static_assert(sonar_pins::BOAT_UART_RX != sonar_pins::BOAT_UART_TX &&
              sonar_pins::BOAT_UART_RX != sonar_pins::IO1 &&
              sonar_pins::BOAT_UART_RX != sonar_pins::IO2 &&
              sonar_pins::BOAT_UART_RX != sonar_pins::OUT3 &&
              sonar_pins::BOAT_UART_RX != sonar_pins::OUT4 &&
              sonar_pins::BOAT_UART_RX != sonar_pins::TUSS_CS &&
              sonar_pins::BOAT_UART_RX != sonar_pins::SPI_SCK &&
              sonar_pins::BOAT_UART_RX != sonar_pins::SPI_MISO &&
              sonar_pins::BOAT_UART_RX != sonar_pins::SPI_MOSI &&
              sonar_pins::BOAT_UART_RX != sonar_pins::VOUT_ADC,
              "Sonar UART RX must be unique in the DevKitC GPIO map");
static_assert(!sonar_pins::isClassicEsp32StrapPin(sonar_pins::BOAT_UART_RX) &&
              !sonar_pins::isClassicEsp32FlashPin(sonar_pins::BOAT_UART_RX) &&
              !sonar_pins::isInputOnlyClassicEsp32Gpio(sonar_pins::BOAT_UART_RX),
              "Sonar UART RX must be a non-strap, output-capable GPIO");
