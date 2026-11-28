#include "esp32_adc_continuous_capture.h"

#if FLYTEC_SONAR_ADC_DRIVER_COMPILE

#include <soc/soc_caps.h>
#include "sonar_pin_map.h"

namespace {
constexpr uint32_t FRAME_CONVERSION_COUNT = 32U;
}

bool Esp32AdcContinuousCapture::begin(bool commissioningApproved) {
  stop();
  lastError_ = ESP_OK;
  poolOverflow_ = false;
  capture_.begin(micros());

  // This candidate is permanently denied in the current source state. A
  // caller flag alone cannot arm ADC capture before physical pin signoff.
  if (!commissioningApproved || !sonar_pins::VOUT_ADC_AUDIT_SIGNED) {
    lastError_ = ESP_ERR_INVALID_STATE;
    capture_.failDriver();
    return false;
  }

  adc_unit_t unit = ADC_UNIT_1;
  if (adc_continuous_io_to_channel(sonar_pins::VOUT_ADC, &unit, &channel_) != ESP_OK ||
      unit != ADC_UNIT_1) {
    lastError_ = ESP_ERR_INVALID_ARG;
    capture_.begin(micros());
    capture_.failDriver();
    return false;
  }

  adc_continuous_handle_cfg_t handleConfig{};
  handleConfig.max_store_buf_size = ADC_POOL_BYTES;
  handleConfig.conv_frame_size = SOC_ADC_DIGI_DATA_BYTES_PER_CONV * FRAME_CONVERSION_COUNT;
  handleConfig.flags.flush_pool = 0; // Never silently replace old sonar samples.
  lastError_ = adc_continuous_new_handle(&handleConfig, &handle_);
  if (lastError_ != ESP_OK) return setupFailed();

  adc_digi_pattern_config_t pattern{};
  pattern.atten = ADC_ATTEN_DB_12;
  pattern.channel = channel_;
  pattern.unit = unit;
  pattern.bit_width = ADC_BITWIDTH_12;

  adc_continuous_config_t adcConfig{};
  adcConfig.pattern_num = 1U;
  adcConfig.adc_pattern = &pattern;
  adcConfig.sample_freq_hz = SAMPLE_RATE_HZ;
  adcConfig.conv_mode = ADC_CONV_SINGLE_UNIT_1;
  adcConfig.format = ADC_DIGI_OUTPUT_FORMAT_TYPE1;
  lastError_ = adc_continuous_config(handle_, &adcConfig);
  if (lastError_ != ESP_OK) return setupFailed();

  adc_continuous_evt_cbs_t callbacks{};
  callbacks.on_pool_ovf = onPoolOverflow;
  lastError_ = adc_continuous_register_event_callbacks(handle_, &callbacks, this);
  if (lastError_ != ESP_OK) return setupFailed();

  capture_.begin(micros());
  lastError_ = adc_continuous_start(handle_);
  if (lastError_ != ESP_OK) {
    capture_.failDriver();
    return setupFailed();
  }
  running_ = true;
  return true;
}

Esp32AdcContinuousCapture::PollResult Esp32AdcContinuousCapture::poll() {
  if (!running_ || !handle_) return PollResult::IDLE;
  if (poolOverflow_) {
    capture_.failOverflow();
    lastError_ = ESP_ERR_INVALID_STATE;
    releaseDriver();
    return PollResult::FAILED;
  }

  uint32_t bytesRead = 0;
  lastError_ = adc_continuous_read(handle_, rawFrame_, sizeof(rawFrame_),
                                   &bytesRead, 0U);
  if (lastError_ == ESP_ERR_TIMEOUT) return PollResult::NO_DATA;
  if (lastError_ != ESP_OK) return fail(lastError_);
  if (bytesRead == 0U || bytesRead > sizeof(rawFrame_) ||
      bytesRead % SOC_ADC_DIGI_RESULT_BYTES != 0U) {
    return fail(ESP_ERR_INVALID_SIZE);
  }

  uint32_t decodedCount = 0;
  lastError_ = adc_continuous_parse_data(handle_, rawFrame_, bytesRead, decoded_,
                                         &decodedCount);
  if (lastError_ != ESP_OK || decodedCount > (sizeof(decoded_) / sizeof(decoded_[0])))
    return fail(lastError_ == ESP_OK ? ESP_ERR_INVALID_SIZE : lastError_);

  for (uint32_t i = 0; i < decodedCount; ++i) {
    const auto &sample = decoded_[i];
    if (!sample.valid || sample.unit != ADC_UNIT_1 || sample.channel != channel_ ||
        sample.raw_data > sonar_capture::ADC_RAW_MAX) {
      capture_.failInvalidSample();
      releaseDriver();
      return PollResult::FAILED;
    }
    const uint16_t raw = static_cast<uint16_t>(sample.raw_data);
    if (!capture_.append(&raw, 1U)) {
      const auto summary = capture_.summary();
      releaseDriver();
      return summary.overflow || summary.invalidSample ? PollResult::FAILED
                                                       : PollResult::IDLE;
    }
    if (capture_.summary().sampleCount == sonar_capture::SAMPLE_CAPACITY) {
      if (poolOverflow_) {
        capture_.failOverflow();
        lastError_ = ESP_ERR_INVALID_STATE;
        releaseDriver();
        return PollResult::FAILED;
      }
      if (!capture_.finish(micros())) return fail(ESP_ERR_INVALID_STATE);
      releaseDriver();
      return capture_.summary().complete ? PollResult::COMPLETE : PollResult::FAILED;
    }
  }
  return poolOverflow_ ? fail(ESP_ERR_INVALID_STATE) : PollResult::CAPTURING;
}

void Esp32AdcContinuousCapture::stop() {
  if (running_) capture_.failDriver();
  releaseDriver();
}

bool Esp32AdcContinuousCapture::onPoolOverflow(
    adc_continuous_handle_t, const adc_continuous_evt_data_t *, void *context) {
  auto *self = static_cast<Esp32AdcContinuousCapture *>(context);
  if (self) self->poolOverflow_ = true;
  return false;
}

void Esp32AdcContinuousCapture::releaseDriver() {
  if (!handle_) {
    running_ = false;
    return;
  }
  if (running_) {
    const esp_err_t stopError = adc_continuous_stop(handle_);
    running_ = false;
    if (stopError != ESP_OK) {
      lastError_ = stopError;
      capture_.failDriver();
    }
  }
  const esp_err_t deinitError = adc_continuous_deinit(handle_);
  handle_ = nullptr;
  if (deinitError != ESP_OK) {
    lastError_ = deinitError;
    capture_.failDriver();
  }
}

bool Esp32AdcContinuousCapture::setupFailed() {
  capture_.failDriver();
  releaseDriver();
  return false;
}

Esp32AdcContinuousCapture::PollResult Esp32AdcContinuousCapture::fail(esp_err_t error) {
  lastError_ = error;
  if (poolOverflow_) capture_.failOverflow();
  else capture_.failDriver();
  releaseDriver();
  return PollResult::FAILED;
}

#endif // FLYTEC_SONAR_ADC_DRIVER_COMPILE
