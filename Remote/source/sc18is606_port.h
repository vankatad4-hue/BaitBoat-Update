#pragma once
// R30H-11: NXP SC18IS606 I2C->SPI transport. NO ESP SPI or GPIO allocation.
// Pure interface for deterministic host fault tests. Never assumes hardware signoff.
#include <stddef.h>
#include <stdint.h>

class Sc18is606Io {
public:
    virtual ~Sc18is606Io() = default;
    virtual bool write(uint8_t address,const uint8_t*bytes,size_t count)=0;
    virtual bool read(uint8_t address,uint8_t*bytes,size_t count)=0;
    virtual void pauseMs(unsigned duration)=0;
};
class Sc18is606Port {
public:
    explicit Sc18is606Port(Sc18is606Io& io,uint8_t address=0x28)
        :io_(io),address_(address){}
    bool begin();             // verifies chip ID; CE held low
    bool ce(bool high);       // SS1/GPIO1 -> nRF CE (requires external pulldown)
    bool transfer(const uint8_t*tx,uint8_t*rx,size_t size); // SS0 -> nRF CSN
    bool ready() const{return ready_;}
    void disable();           // best-effort CE low; untrusted on I2C failure
    static constexpr uint8_t DEFAULT_ADDRESS=0x28;
    static constexpr uint8_t SS0_SELECT=0x01;
    static constexpr size_t MAX_TRANSFER=33; // command + 32-byte RF payload
private:
    bool command(const uint8_t*data,size_t count,bool retry);
    bool fetch(uint8_t*data,size_t count);
    Sc18is606Io& io_;
    uint8_t address_;
    bool ready_=false;
};
