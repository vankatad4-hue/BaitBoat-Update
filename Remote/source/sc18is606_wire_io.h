#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "sc18is606_port.h"
// Adapter reuses board-owned Wire; never calls Wire.begin()/setClock().
class Sc18is606WireIo final : public Sc18is606Io {
public:
    explicit Sc18is606WireIo(TwoWire&wire):wire_(wire){}
    bool write(uint8_t addr,const uint8_t*bytes,size_t count) override;
    bool read(uint8_t addr,uint8_t*bytes,size_t count) override;
    uint32_t tickMs() const override{return millis();}
    void pauseMs(unsigned duration) override{delay(duration);}
private:
    TwoWire&wire_;
};
