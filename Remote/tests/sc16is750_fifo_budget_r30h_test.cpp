#include "../source/sc16is750_fifo_budget.h"
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    using namespace r30h_sc16;
    static_assert(FIFO_CAPACITY == 64, "SC16IS750 TX FIFO depth");
    // TXLVL=64: FIFO empty, ALL 64 bytes may be queued; before R30H writeFifo
    // incorrectly computed 64 - TXLVL and could not send any E22 frame.
    assert(freeTxSlots(0x40) == 64);
    assert(txFifoEmpty(0x40));
    assert(freeTxSlots(0x3F) == 63);
    assert(freeTxSlots(0x20) == 32);
    assert(freeTxSlots(0x01) == 1);
    assert(freeTxSlots(0x00) == 0);
    assert(!txFifoEmpty(0x00)); // full is NOT drained
    // Unexpected bit7/high data must NOT wrap and authorize a write.
    assert(freeTxSlots(0x41) == 0);
    assert(freeTxSlots(0xFF) == 0);
    assert(!txFifoEmpty(0xFF));
    std::cout << "R30H SC16IS750 TXLVL semantics passed (host-only)\n";
}
