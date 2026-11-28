#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Format a validated depth as the compact NMEA DBT sentence consumed by
// Boat's OpenEchoDbtDecoder. This helper has no serial or sensor side effects.
// Range matches Boat's current parser policy: 0.50 m through 100.00 m.
inline bool openEchoFormatDbt(uint16_t depthCm, char *dst, size_t capacity) {
  if (!dst || capacity == 0U || depthCm < 50U || depthCm > 10000U) return false;

  // Use integer arithmetic so formatting is deterministic on the small MCU.
  const uint32_t feetTenths = ((uint32_t)depthCm * 328084UL + 500000UL) / 1000000UL;
  const uint32_t fathomsTenths = ((uint32_t)depthCm * 1000UL + 9144UL) / 18288UL;
  char body[40];
  const int bodyLength = snprintf(body, sizeof(body), "SDDBT,%lu.%lu,f,%u.%02u,M,%lu.%lu,F",
      (unsigned long)(feetTenths / 10U), (unsigned long)(feetTenths % 10U),
      (unsigned)(depthCm / 100U), (unsigned)(depthCm % 100U),
      (unsigned long)(fathomsTenths / 10U), (unsigned long)(fathomsTenths % 10U));
  if (bodyLength <= 0 || (size_t)bodyLength >= sizeof(body)) return false;

  // NMEA checksum is XOR of every body byte between '$' and '*'.
  uint8_t checksum = 0;
  for (int i = 0; i < bodyLength; ++i) checksum ^= (uint8_t)body[i];
  const int written = snprintf(dst, capacity, "$%s*%02X\r\n", body, checksum);
  return written > 0 && (size_t)written < capacity;
}
