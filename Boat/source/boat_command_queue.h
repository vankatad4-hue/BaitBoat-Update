#pragma once
#include "boat_commands.h"
#include <stdint.h>

// Fixed-capacity FIFO for decoded commands. Link is stored as a small numeric
// identifier to keep this utility independent of radio-driver headers.
class BoatCommandQueue {
public:
    static constexpr uint8_t CAPACITY=4;
    BoatCommandQueue();
    void clear();
    // Drop queued commands from a lost link, retaining an emergency STOP.
    void discardLink(uint8_t link,bool keepEmergencyStop=true);
    // Drop every queued action except an already received emergency STOP.
    void clearNonEmergency();
    bool push(const BoatCommand &cmd,uint8_t link,bool duplicate,uint8_t epoch);
    bool pop(BoatCommand &cmd,bool &duplicate,uint8_t &link,uint8_t &epoch);
    bool contains(uint8_t epoch,uint16_t commandId) const;
    uint8_t size() const { return count_; }
    bool full() const { return count_>=CAPACITY; }
private:
    struct Entry { BoatCommand command; uint8_t link; bool duplicate; uint8_t epoch; };
    Entry entries_[CAPACITY];
    uint8_t write_,read_,count_;
};
