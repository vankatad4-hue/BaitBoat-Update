#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "lora_link_policy.h"

class SC16IS750Bridge : public Stream {
public:
    explicit SC16IS750Bridge(TwoWire &wire=Wire, uint8_t address=0x48);
    bool begin(uint32_t baud=9600);
    bool ping();
    // Commissioning-only; normal UART Stream remains disabled after lock.
    bool lockForCommissioning(const flytec_rtos::E22CommissioningPermit& permit,
                              flytec_rtos::E22IsolationProbe isolation,void* context);
    bool commissioningBaudVerified(uint32_t crystalHz,uint32_t baud,uint32_t limitMs);
    bool setGpioDirection(uint8_t directionMask);
    bool setGpioState(uint8_t outputState);
    bool readGpio(uint8_t &state);
    bool setE22Mode(bool m0, bool m1); // GPIO0=M0, GPIO1=M1; config is M0=0 M1=1
    bool setE22Reset(bool released);
    bool e22AuxReady();

    int availableForWrite() override;
    int available() override;
    int read() override;
    int peek() override;
    void flush() override;
    size_t write(uint8_t value) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    using Print::write;

private:
    TwoWire &wire_;
    uint8_t address_;
    int peekByte_;
    uint8_t rxLevelCache_;
    bool ready_;
    bool commissioningLocked_=false,commissioningFault_=false;
    flytec_rtos::E22IsolationProbe isolation_=nullptr;void* isolationContext_=nullptr;
    friend class flytec_rtos::E22OfflinePort<SC16IS750Bridge>;
    void commissioningLatchFault(){commissioningFault_=true;}
    uint32_t commissioningNow()const{return millis();}
    void commissioningWait(){delay(1);}
    bool commissioningHostBaud(uint32_t crystal,uint32_t baud,uint32_t limit){return commissioningBaudVerified(crystal,baud,limit);}
    bool commissioningModeMatches(bool config);
    bool commissioningModeVerified(bool config,uint32_t limit);
    bool commissioningRxCount(unsigned& count);
    bool commissioningReadByte(uint8_t& value);
    bool commissioningTxIdle();
    bool commissioningConfigWrite(const uint8_t* cmd,unsigned n);
    bool commissioningAllowed()const;
    bool writeReg(uint8_t reg, uint8_t value);
    bool readReg(uint8_t reg, uint8_t &value);
    size_t writeFifo(const uint8_t *data, size_t length);
    size_t readFifo(uint8_t *data, size_t length);
    uint8_t i2cSubAddress(uint8_t reg) const { return (uint8_t)(reg << 3); }
};
