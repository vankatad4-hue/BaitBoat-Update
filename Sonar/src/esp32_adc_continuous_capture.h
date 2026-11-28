#pragma once

#ifndef FLYTEC_SONAR_ADC_DRIVER_COMPILE
#define FLYTEC_SONAR_ADC_DRIVER_COMPILE 0
#endif

#if FLYTEC_SONAR_ADC_DRIVER_COMPILE

#include <Arduino.h>
#include <esp_adc/adc_continuous.h>
#include <soc/soc_caps.h>
#include "adc_capture_buffer.h"

// ESP32 classic ADC1/DMA acquisition adapter. It is compiled only in the
// dedicated API-compile environment and cannot start until the VOUT pin audit
// is changed by a separately reviewed hardware commissioning step.
class Esp32AdcContinuousCapture {
 public:
  enum class PollResult : uint8_t {
    IDLE,
    NO_DATA,
    CAPTURING,
    COMPLETE,
    FAILED,
  };

  Esp32AdcContinuousCapture() = default;
  Esp32AdcContinuousCapture(const Esp32AdcContinuousCapture &) = delete;
  Esp32AdcContinuousCapture &operator=(const Esp32AdcContinuousCapture &) = delete;

  // Driver setup is harmless until the VOUT pin audit constant is signed.
  bool begin(bool commissioningApproved);
  PollResult poll();
  void stop();

  const sonar_capture::AdcCaptureBuffer &capture() const { return capture_; }
  esp_err_t lastError() const { return lastError_; }
  bool running() const { return running_; }

 private:
  static constexpr uint32_t SAMPLE_RATE_HZ = 75800U; // Candidate: 13.2 us/sample.
  static constexpr uint32_t ADC_POOL_BYTES = 32768U;
  static constexpr uint32_t FRAME_BYTES = 256U;

  static bool onPoolOverflow(adc_continuous_handle_t handle,
                             const adc_continuous_evt_data_t *event,
                             void *context);
  bool setupFailed();
  void releaseDriver();
  PollResult fail(esp_err_t error);

  adc_continuous_handle_t handle_ = nullptr;
  adc_channel_t channel_{};
  sonar_capture::AdcCaptureBuffer capture_{};
  uint8_t rawFrame_[FRAME_BYTES]{};
  adc_continuous_data_t decoded_[FRAME_BYTES / SOC_ADC_DIGI_RESULT_BYTES]{};
  volatile bool poolOverflow_ = false;
  bool running_ = false;
  esp_err_t lastError_ = ESP_OK;
};

#endif // FLYTEC_SONAR_ADC_DRIVER_COMPILE
