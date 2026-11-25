#pragma once
// R30H-12: theoretical MINIMUM bus occupancy, NOT a latency bound or signoff.
// No GPIO, no I2C activity. Counts 9 bits per I2C address/data byte;
// excludes START/STOP, clock stretching, RTOS scheduling, latency and retries.
#include <stdint.h>
namespace r30h12_wire {
constexpr uint32_t bitsForWrite(uint32_t bytes){return 9U*(1U+bytes);}
constexpr uint32_t bitsForRead(uint32_t bytes){return 9U*(1U+bytes);}
constexpr uint32_t bridgeTransferBits(uint32_t spiBytes){
    // Function ID 0x01 plus spiBytes over I2C write, read spiBytes back.
    return bitsForWrite(spiBytes+1U)+bitsForRead(spiBytes);
}
constexpr uint32_t mcpCeWriteBits(){
    // read OLATA (register pointer write + one byte read), write OLATA,
    // read GPIOA for physical pin echo. GPA4 OK configuration preserved.
    return bitsForWrite(1U)+bitsForRead(1U)+bitsForWrite(2U)+
           bitsForWrite(1U)+bitsForRead(1U);
}
constexpr uint32_t sendFrameWireBits(){
    // CE low; CONFIG, STATUS and FLUSH_TX commands; 32-byte W_TX_PAYLOAD;
    // CE high then low. Applies only to this source's nominal transaction path.
    return 3U*mcpCeWriteBits()+2U*bridgeTransferBits(2U)+
           bridgeTransferBits(1U)+bridgeTransferBits(33U);
}
constexpr uint32_t completedTxAndTwoRxPollBits(){
    // NOP STATUS; STATUS clear; return-to-RX CE low/CONFIG/CE high;
    // FIFO_STATUS read; two RX frames and FIFO rechecks.
    return bridgeTransferBits(1U)+bridgeTransferBits(2U)+
           2U*mcpCeWriteBits()+bridgeTransferBits(2U)+
           bridgeTransferBits(2U)+2U*(bridgeTransferBits(33U)+
                                      bridgeTransferBits(2U));
}
constexpr uint32_t ceilWireUs(uint32_t bits,uint32_t i2cHz){
    return i2cHz==0U?0U:
           (uint32_t)(((uint64_t)bits*1000000ULL+i2cHz-1U)/i2cHz);
}
static_assert(bridgeTransferBits(33U)==621U,"32B nRF I2C bus wire accounting");
static_assert(mcpCeWriteBits()==99U,"MCP CE read-modify-write accounting");
static_assert(sendFrameWireBits()==1089U,"nRF TX minimum I2C wire floor");
static_assert(completedTxAndTwoRxPollBits()==1800U,"nRF poll/RX wire floor");
} // namespace r30h12_wire
