#pragma once
// R30H-11: NXP SC18IS606 I2C->SPI transport. NO ESP SPI or GPIO allocation.
// Pure interface for deterministic host fault tests. Never assumes hardware signoff.
#include <stddef.h>
#include <stdint.h>
#include "mcp23017_ce_port.h"

class Sc18is606Io {
public:
    virtual ~Sc18is606Io() = default;
    virtual bool write(uint8_t address,const uint8_t*bytes,size_t count)=0;
    virtual bool read(uint8_t address,uint8_t*bytes,size_t count)=0;
    virtual uint32_t tickMs() const=0; // monotonically wrapping millisecond timer
    virtual void pauseMs(unsigned duration)=0;
};
class Sc18is606Port {
public:
    explicit Sc18is606Port(Sc18is606Io& io,Mcp23017CePort& ce,uint8_t address=0x28)
        :io_(io),ce_(ce),address_(address){}
    bool begin();             // verifies CE-low on MCP23017 then bridge identity
    bool ce(bool high);      // MCP23017@0x21 GPA0, NOT bridge SS2
    bool transfer(const uint8_t*tx,uint8_t*rx,size_t size); // SS0 -> nRF CSN
    bool ready() const{return ready_;}
    void disable();           // best-effort MCP CE low; I2C failure untrusted
    static constexpr uint8_t DEFAULT_ADDRESS=0x28;
    static constexpr uint8_t SS0_SELECT=0x01;
    static constexpr size_t MAX_TRANSFER=33; // command + 32-byte RF payload
    // Software elapsed-time guard, NOT an MCU/physical bus watchdog.
    // A single blocking Wire call may exceed this value before returning.
    static constexpr uint32_t OPERATION_BUDGET_MS=12;
private:
    bool command(const uint8_t*data,size_t count,bool retry);
    bool fetch(uint8_t*data,size_t count);
    Sc18is606Io& io_;
    Mcp23017CePort& ce_;
    uint8_t address_;
    bool ready_=false;
};
