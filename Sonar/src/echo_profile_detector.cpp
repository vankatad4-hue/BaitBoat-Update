#include "echo_profile_detector.h"

namespace {
constexpr uint8_t MAX_CONSECUTIVE_SAMPLES = 32U;
constexpr uint32_t MIN_SOUND_SPEED_MM_PER_SECOND = 1000000U;
constexpr uint32_t MAX_SOUND_SPEED_MM_PER_SECOND = 2000000U;
constexpr uint32_t MAX_SAMPLE_INTERVAL_NS = 100000U;
constexpr uint64_t DEPTH_CM_DENOMINATOR = 20000000000ULL;
constexpr uint64_t DEPTH_CM_ROUNDING = DEPTH_CM_DENOMINATOR / 2U;
constexpr uint16_t MIN_REPORTABLE_DEPTH_CM = 50U;
constexpr uint16_t MAX_REPORTABLE_DEPTH_CM = 10000U;
}

EchoDetectionResult EchoProfileDetector::detect(
    const sonar_capture::AdcCaptureBuffer &capture,
    const EchoDetectorConfig &config) const {
  const auto summary = capture.summary();
  if (!summary.complete || summary.overflow || summary.invalidSample ||
      summary.driverFailure || summary.sampleCount != sonar_capture::SAMPLE_CAPACITY) {
    return {EchoDetectionStatus::INVALID_CAPTURE, 0U, 0U, 0U};
  }

  if (config.blankUntilSample >= summary.sampleCount ||
      config.thresholdAdcRaw == 0U ||
      config.thresholdAdcRaw > sonar_capture::ADC_RAW_MAX ||
      config.consecutiveSamples == 0U ||
      config.consecutiveSamples > MAX_CONSECUTIVE_SAMPLES ||
      config.sampleIntervalNs == 0U ||
      config.sampleIntervalNs > MAX_SAMPLE_INTERVAL_NS ||
      config.soundSpeedMmPerSecond < MIN_SOUND_SPEED_MM_PER_SECOND ||
      config.soundSpeedMmPerSecond > MAX_SOUND_SPEED_MM_PER_SECOND) {
    return {EchoDetectionStatus::INVALID_CONFIG, 0U, 0U, 0U};
  }

  const uint16_t *samples = capture.samples();
  const size_t lastStart = summary.sampleCount - config.consecutiveSamples;
  for (size_t start = config.blankUntilSample; start <= lastStart; ++start) {
    uint16_t peak = 0U;
    bool stableCrossing = true;
    for (size_t offset = 0; offset < config.consecutiveSamples; ++offset) {
      const uint16_t raw = samples[start + offset];
      if (raw > sonar_capture::ADC_RAW_MAX) {
        return {EchoDetectionStatus::INVALID_CAPTURE, 0U, 0U, 0U};
      }
      if (raw < config.thresholdAdcRaw) stableCrossing = false;
      if (raw > peak) peak = raw;
    }
    if (!stableCrossing) continue;

    // depth_cm = sample * interval_ns * speed_mm_s / (2 * 10 * 1e9).
    const uint64_t numerator = static_cast<uint64_t>(start) *
                               config.sampleIntervalNs *
                               config.soundSpeedMmPerSecond;
    const uint64_t roundedDepthCm =
        (numerator + DEPTH_CM_ROUNDING) / DEPTH_CM_DENOMINATOR;
    if (roundedDepthCm < MIN_REPORTABLE_DEPTH_CM ||
        roundedDepthCm > MAX_REPORTABLE_DEPTH_CM) {
      return {EchoDetectionStatus::DEPTH_OUT_OF_RANGE,
              static_cast<uint16_t>(start),
              static_cast<uint16_t>(roundedDepthCm > UINT16_MAX
                                        ? UINT16_MAX
                                        : roundedDepthCm),
              peak};
    }
    return {EchoDetectionStatus::FOUND, static_cast<uint16_t>(start),
            static_cast<uint16_t>(roundedDepthCm), peak};
  }
  return {EchoDetectionStatus::NO_ECHO, 0U, 0U, 0U};
}
