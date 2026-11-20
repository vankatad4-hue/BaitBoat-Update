#ifndef OFFLINE_MAP_COVERAGE_H
#define OFFLINE_MAP_COVERAGE_H
#include <Arduino.h>
struct CoverageTile { uint8_t z; uint32_t x,y; uint8_t present; };
bool mapCoverage_begin();
bool mapCoverage_hasIndex();
bool mapCoverage_tilePresent(uint8_t z,uint32_t x,uint32_t y);
void mapCoverage_getStats(uint16_t &planned,uint16_t &present,uint16_t &missing);
bool mapCoverage_calculationCapped();
void mapCoverage_checkSquare(double lat,double lon,uint16_t sideM,const uint8_t *zooms,uint8_t zoomCount,uint16_t &needed,uint16_t &covered,uint16_t &missing);
#endif
