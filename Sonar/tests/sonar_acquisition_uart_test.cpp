#include <array>
#include <cassert>
#include <cstdint>
#include "adc_capture_buffer.h"
#include "dbt_sentence.h"
#include "echo_profile_detector.h"
#include "open_echo_dbt.h"
#include "sonar_pin_map.h"

int main() {
  // Keep the written GPIO/UART allocation synchronized with the build target.
  static_assert(sonar_pins::IO1 == 25 && sonar_pins::IO2 == 26 &&
                    sonar_pins::OUT3 == 27 && sonar_pins::OUT4 == 32,
                "Shield GPIO assignments changed; update the hardware crosswalk");
  static_assert(sonar_pins::TUSS_CS == 21 && sonar_pins::SPI_SCK == 18 &&
                    sonar_pins::SPI_MISO == 19 && sonar_pins::SPI_MOSI == 23,
                "TUSS4470 SPI allocation changed; update the bus crosswalk");
  static_assert(sonar_pins::VOUT_ADC == 34 && sonar_pins::BOAT_UART_TX == 22,
                "ADC/UART allocation changed; update the endpoint crosswalk");

  // A full DMA-equivalent sample block is required before depth detection.
  std::array<uint16_t, sonar_capture::SAMPLE_CAPACITY> samples{};
  samples[1300] = 2400U;
  samples[1301] = 2500U;
  samples[1302] = 2300U;

  sonar_capture::AdcCaptureBuffer capture;
  capture.begin(0xFFFFFF00U);
  assert(capture.append(samples.data(), samples.size()));
  assert(capture.finish(0x00000100U)); // Elapsed time must survive uint32 wrap.
  assert(capture.summary().durationUs == 512U);

  const EchoDetectorConfig config{0U, 40U, 2000U, 3U, 13200U, 1500000U};
  const EchoProfileDetector detector;
  const auto found = detector.detect(capture, config);
  assert(found.status == EchoDetectionStatus::FOUND);
  assert(found.sampleIndex == 1300U);
  assert(found.depthCm == 1287U);
  assert(found.peakAdcRaw == 2500U);

  // A single high sample must not qualify as a stable echo crossing.
  samples[1301] = 0U;
  sonar_capture::AdcCaptureBuffer noisy;
  noisy.begin(10U);
  assert(noisy.append(samples.data(), samples.size()));
  assert(noisy.finish(20U));
  assert(detector.detect(noisy, config).status == EchoDetectionStatus::NO_ECHO);

  // Incomplete and malformed captures fail closed and never yield a depth.
  sonar_capture::AdcCaptureBuffer incomplete;
  incomplete.begin(0U);
  assert(incomplete.append(samples.data(), samples.size() - 1U));
  assert(!incomplete.finish(1U));
  assert(detector.detect(incomplete, config).status ==
         EchoDetectionStatus::INVALID_CAPTURE);

  std::array<uint16_t, sonar_capture::SAMPLE_CAPACITY + 1U> tooMany{};
  sonar_capture::AdcCaptureBuffer overflow;
  overflow.begin(0U);
  assert(!overflow.append(tooMany.data(), tooMany.size()));
  assert(overflow.summary().overflow);

  samples[1301] = 2500U;
  samples[1302] = 5000U;
  sonar_capture::AdcCaptureBuffer invalid;
  invalid.begin(0U);
  assert(!invalid.append(samples.data(), samples.size()));
  assert(invalid.summary().invalidSample);

  // Validate the encoded 9600-8N1 DBT sentence through Boat's real parser.
  char sentence[80]{};
  assert(openEchoFormatDbt(1287U, sentence, sizeof(sentence)));
  OpenEchoDbtDecoder decoder;
  uint16_t decodedCm = 0U;
  bool decoded = false;
  uint32_t now = 0U;
  for (const char byte : sentence) {
    if (byte == '\0') break;
    decoded = decoder.feed(byte, now++, decodedCm) || decoded;
  }
  assert(decoded);
  assert(decodedCm == 1287U);

  // Tampered UART data must fail the NMEA checksum and produce no depth.
  sentence[10] = sentence[10] == '9' ? '8' : '9';
  decoder.reset();
  decoded = false;
  for (const char byte : sentence) {
    if (byte == '\0') break;
    decoded = decoder.feed(byte, now++, decodedCm) || decoded;
  }
  assert(!decoded);
}
