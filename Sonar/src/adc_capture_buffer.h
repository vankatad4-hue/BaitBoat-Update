#pragma once

#include <stddef.h>
#include <stdint.h>

// Fixed-size staging buffer for one Open Echo raw capture. This component has
// no ADC, GPIO, timer, UART, Wi-Fi, or transducer access; a future ADC adapter
// may append samples here and must reject any block that overflows it.
namespace sonar_capture {

// Open Echo's reference capture uses 5,224 samples. At 16 bits per stored
// sample this reserves 10,448 bytes, independent of heap availability.
constexpr size_t SAMPLE_CAPACITY = 5224U;
constexpr uint16_t ADC_RAW_MAX = 4095U; // Classic ESP32 12-bit raw ADC limit.

struct CaptureSummary {
  size_t sampleCount;
  uint32_t durationUs;
  bool complete;
  bool overflow;
  bool invalidSample;
  bool driverFailure;
};

class AdcCaptureBuffer {
 public:
  // Start a new capture and clear the previous result.
  void begin(uint32_t startedAtUs) {
    sampleCount_ = 0;
    startedAtUs_ = startedAtUs;
    durationUs_ = 0;
    active_ = true;
    complete_ = false;
    overflow_ = false;
    invalidSample_ = false;
    driverFailure_ = false;
  }

  // Copy an ADC block only if the entire block fits and every sample is valid.
  // Rejecting the whole block prevents a partial overrun from looking complete.
  bool append(const uint16_t *samples, size_t count) {
    if (!active_) return false;
    if (!samples && count != 0U) {
      driverFailure_ = true;
      active_ = false;
      complete_ = false;
      return false;
    }
    if (count > SAMPLE_CAPACITY - sampleCount_) {
      overflow_ = true;
      active_ = false;
      complete_ = false;
      return false;
    }
    for (size_t i = 0; i < count; ++i) {
      if (samples[i] > ADC_RAW_MAX) {
        invalidSample_ = true;
        active_ = false;
        complete_ = false;
        return false;
      }
    }
    for (size_t i = 0; i < count; ++i) samples_[sampleCount_ + i] = samples[i];
    sampleCount_ += count;
    return true;
  }

  // Preserve the reason a driver-side ADC pool overflow invalidated capture.
  void failOverflow() {
    overflow_ = true;
    active_ = false;
    complete_ = false;
  }

  // Reject a frame with invalid channel metadata or malformed ADC records.
  void failInvalidSample() {
    invalidSample_ = true;
    active_ = false;
    complete_ = false;
  }

  // Mark a peripheral/configuration failure without presenting a partial block.
  void failDriver() {
    driverFailure_ = true;
    active_ = false;
    complete_ = false;
  }

  // Finish only after the exact reference sample count has arrived.
  bool finish(uint32_t endedAtUs) {
    if (!active_) return false;
    active_ = false;
    durationUs_ = static_cast<uint32_t>(endedAtUs - startedAtUs_);
    complete_ = sampleCount_ == SAMPLE_CAPACITY && !overflow_ &&
                 !invalidSample_ && !driverFailure_;
    return complete_;
  }

  const uint16_t *samples() const { return samples_; }

  CaptureSummary summary() const {
    return {sampleCount_, durationUs_, complete_, overflow_, invalidSample_,
            driverFailure_};
  }

 private:
  uint16_t samples_[SAMPLE_CAPACITY]{};
  size_t sampleCount_ = 0;
  uint32_t startedAtUs_ = 0;
  uint32_t durationUs_ = 0;
  bool active_ = false;
  bool complete_ = false;
  bool overflow_ = false;
  bool invalidSample_ = false;
  bool driverFailure_ = false;
};

} // namespace sonar_capture
