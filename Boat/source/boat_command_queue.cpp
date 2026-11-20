#include "boat_command_queue.h"
#include <string.h>

BoatCommandQueue::BoatCommandQueue() : write_(0),read_(0),count_(0) { clear(); }
void BoatCommandQueue::clear() {
    memset(entries_,0,sizeof(entries_));
    write_=read_=count_=0;
}
bool BoatCommandQueue::push(const BoatCommand &cmd,uint8_t link,bool duplicate,uint8_t epoch) {
    if(full()) return false;
    entries_[write_]={cmd,link,duplicate,epoch};
    write_=(uint8_t)((write_+1U)%CAPACITY);
    ++count_;
    return true;
}
bool BoatCommandQueue::pop(BoatCommand &cmd,bool &duplicate,uint8_t &link,uint8_t &epoch) {
    if(count_==0) return false;
    const Entry e=entries_[read_];
    read_=(uint8_t)((read_+1U)%CAPACITY);
    --count_;
    cmd=e.command; duplicate=e.duplicate; link=e.link; epoch=e.epoch;
    return true;
}
bool BoatCommandQueue::contains(uint8_t epoch,uint16_t commandId) const {
    for(uint8_t i=0;i<count_;++i) {
        const uint8_t index=(uint8_t)((read_+i)%CAPACITY);
        const Entry &e=entries_[index];
        if(e.epoch==epoch && (uint16_t)e.command.commandId==commandId) return true;
    }
    return false;
}
