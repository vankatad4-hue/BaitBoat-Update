#pragma once

#include <stdint.h>
#include "adc_capture_buffer.h"

// Pure software classifier for a completed VOUT envelope capture. It has no
// peripheral access and deliberately has no default threshold/calibration.
struct EchoDetectorConfig {
  uint16_t transmitSampleIndex;  // Measured sample coordinate of acoustic burst start.
  uint16_t blankingSamples;      // Ignore this many samples after the burst origin.
  uint16_t thresholdAdcRaw;      // Calibrated VOUT ADC code, not TUSS OUT4 register data.
  uint8_t consecutiveSamples;    // Require a short run to reject single-sample spikes.
  uint32_t sampleIntervalNs;     // Measured conversion interval, not the requested ADC rate.
  uint32_t soundSpeedMmPerSecond;
};

enum class EchoDetectionStatus : uint8_t {
  FOUND,
  NO_ECHO,
  INVALID_CONFIG,
  INVALID_CAPTURE,
  DEPTH_OUT_OF_RANGE,
};

struct EchoDetectionResult {
  EchoDetectionStatus status;
  uint16_t sampleIndex;
  uint16_t depthCm;
  uint16_t peakAdcRaw;
};

class EchoProfileDetector {
 public:
  // Detect the first stable threshold crossing after the configured blanking
  // window. Depth is an estimate from sample index; calibration is external.
  EchoDetectionResult detect(const sonar_capture::AdcCaptureBuffer &capture,
                             const EchoDetectorConfig &config) const;
};
