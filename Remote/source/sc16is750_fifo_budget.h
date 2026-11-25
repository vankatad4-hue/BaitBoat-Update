#pragma once
#include <stdint.h>

// NXP SC16IS750 TXLVL (0x08) returns FREE slots, not bytes occupied.
// 0x40 means empty TX FIFO; 0x00 means full. Invalid reads fail closed.
namespace r30h_sc16 {
constexpr uint8_t FIFO_CAPACITY = 64;
inline uint8_t freeTxSlots(uint8_t txlvl) {
    return txlvl <= FIFO_CAPACITY ? txlvl : 0;
}
inline bool txFifoEmpty(uint8_t txlvl) {
    return txlvl == FIFO_CAPACITY;
}
} // namespace r30h_sc16
