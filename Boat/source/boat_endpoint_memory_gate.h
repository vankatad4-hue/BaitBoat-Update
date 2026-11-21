#pragma once
#include <stdint.h>
// Confirmed Boat endpoint model: ESP32-S3-DevKitC-1-N32R16V.
// A matching memory report is necessary but never sufficient to authorize
// external GPIO, motor, solenoid, RF or independent safety hardware.
static constexpr uint32_t BOAT_N32R16V_FLASH_BYTES=32UL*1024UL*1024UL;
static constexpr uint32_t BOAT_N32R16V_PSRAM_BYTES=16UL*1024UL*1024UL;
inline bool boatEndpointMemoryMatches(uint32_t flashBytes,uint32_t psramBytes){
 return flashBytes==BOAT_N32R16V_FLASH_BYTES &&
        psramBytes==BOAT_N32R16V_PSRAM_BYTES;
}
