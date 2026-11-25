#pragma once
// R30H-9 uncommissioned Boat ESP32-S3-DevKitC-1-N32R16V.
// No spare diagnostic GPIO has been released from the physical STOP/feedback
// reservation. This header deliberately cannot initialize any GPIO.
#define BOAT_R30H9_MARKER_GPIO (-1)
#define BOAT_R30H9_GPIO_OUTPUT_SIGNED 0
#if BOAT_R30H9_MARKER_GPIO != (-1) || BOAT_R30H9_GPIO_OUTPUT_SIGNED != 0
#error "R30H-9 physical marker forbidden: no audited Boat GPIO, STOP/feedback pins reserved"
#endif
