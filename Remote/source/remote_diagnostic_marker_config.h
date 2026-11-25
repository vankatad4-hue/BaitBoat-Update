#pragma once
// R30H-9 Waveshare ESP32-S3-Touch-LCD-4.3 Standard SKU25948.
// The LCD/SD/touch/RS485/Sensor-AD board routes have NOT been inspected on
// the purchased revision. Do not infer an unused MCU pin from generic S3.
#define REMOTE_R30H9_MARKER_GPIO (-1)
#define REMOTE_R30H9_GPIO_OUTPUT_SIGNED 0
#if REMOTE_R30H9_MARKER_GPIO != (-1) || REMOTE_R30H9_GPIO_OUTPUT_SIGNED != 0
#error "R30H-9 physical marker forbidden: no audited Waveshare pin, onboard devices reserve GPIO"
#endif
