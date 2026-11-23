#include "r30h12_i2c_wire_budget.h"
#include <assert.h>
int main(){
    using namespace r30h12_wire;
    static_assert(bridgeTransferBits(1U)==45U,"one SPI byte");
    static_assert(bridgeTransferBits(33U)==621U,"32-byte nRF payload");
    static_assert(mcpCeWriteBits()==99U,"CE read/modify/readback");
    static_assert(sendFrameWireBits()==1089U,"TX I2C wire floor");
    static_assert(completedTxAndTwoRxPollBits()==1800U,"RX I2C wire floor");
    // Values below are optimistic MINIMA ONLY, NOT STOP latency promises.
    assert(ceilWireUs(sendFrameWireBits(),100000U)==10890U);
    assert(ceilWireUs(sendFrameWireBits(),400000U)==2723U);
    assert(ceilWireUs(completedTxAndTwoRxPollBits(),100000U)==18000U);
    assert(ceilWireUs(completedTxAndTwoRxPollBits(),400000U)==4500U);
    assert(ceilWireUs(sendFrameWireBits(),0U)==0U); // no false PASS for unknown clock
    return 0;
}
