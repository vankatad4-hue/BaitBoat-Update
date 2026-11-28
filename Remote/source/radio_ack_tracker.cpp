#include "radio_ack_tracker.h"

RadioAckTracker::RadioAckTracker() { reset(0); }
void RadioAckTracker::reset(uint8_t epoch) {
    epoch_=epoch;
    for(uint8_t i=0;i<CAPACITY;i++){slots_[i]={};slots_[i].used=false;}
}
int RadioAckTracker::find(uint16_t id) const {
    for(uint8_t i=0;i<CAPACITY;i++) if(slots_[i].used && slots_[i].record.commandId==id) return i;
    return -1;
}
int RadioAckTracker::allocate() const {
    for(uint8_t i=0;i<CAPACITY;i++) if(!slots_[i].used) return i;
    for(uint8_t i=0;i<CAPACITY;i++) {
        const RadioCommandAckState s=slots_[i].record.state;
        if(s==RADIO_CMD_ACK_EXECUTED || s==RADIO_CMD_ACK_DUPLICATE ||
           s==RADIO_CMD_ACK_REJECTED || s==RADIO_CMD_ACK_TIMEOUT) return i;
    }
    return -1;
}
bool RadioAckTracker::primaryOnly(uint8_t type,uint8_t mode) const {
    // 2=clear STOP, 3=set mode, 5/6 legacy combined release, 7..10 releases, 11=sonar power.
    // Backup is limited to STOP, RETURN HOME and restricted manual steering.
    if(type==2 || type==5 || type==6 || (type>=7 && type<=11)) return true;
    if(type==3 && mode!=2) return true; // MODE_RETURN_HOME wire value is 2
    return false;
}
bool RadioAckTracker::begin(uint16_t id,uint8_t type,uint8_t mode,uint32_t now,uint16_t timeout) {
    if(id==0 || type<1 || type>11 || timeout==0) return false;
    const int existing=find(id);
    if(existing>=0 && (slots_[existing].record.state==RADIO_CMD_ACK_WAITING ||
                       slots_[existing].record.state==RADIO_CMD_ACK_ACCEPTED)) return false;
    const int index=existing>=0?existing:allocate();
    if(index<0) return false;
    Slot &slot=slots_[index]; slot.used=true; slot.timeoutMs=timeout;
    slot.record={}; slot.record.commandId=id; slot.record.commandType=type;
    slot.record.requestedMode=mode; slot.record.state=RADIO_CMD_ACK_WAITING;
    slot.record.createdAtMs=now; slot.record.updatedAtMs=now;
    return true;
}
bool RadioAckTracker::observe(const RadioDecodedPayload &ack,RadioAckLink link,uint8_t epoch,uint32_t now) {
    if(ack.kind!=RADIO_PAYLOAD_ACK || epoch!=epoch_ ||
       (link!=RADIO_ACK_LINK_PRIMARY && link!=RADIO_ACK_LINK_BACKUP)) return false;
    const int index=find(ack.commandId); if(index<0) return false;
    Slot &slot=slots_[index];
    if(slot.record.commandType!=ack.commandType ||
       (link==RADIO_ACK_LINK_BACKUP && primaryOnly(slot.record.commandType,slot.record.requestedMode))) return false;
    const RadioCommandAckState current=slot.record.state;
    if(current!=RADIO_CMD_ACK_WAITING && current!=RADIO_CMD_ACK_ACCEPTED) return false;
    // Check the immutable request deadline even before periodic update().
    // Unsigned subtraction also rejects captures predating the request.
    if(uint32_t(now-slot.record.createdAtMs)>=slot.timeoutMs ||
       uint32_t(now-slot.record.updatedAtMs)>=0x80000000U) return false;
    slot.record.lastResult=ack.ackResult; slot.record.stopState=ack.stopState;
    slot.record.interlockFlags=ack.interlockFlags; slot.record.updatedAtMs=now;
    switch(ack.ackResult) {
        case RADIO_ACK_ACCEPTED: slot.record.state=RADIO_CMD_ACK_ACCEPTED; break;
        case RADIO_ACK_EXECUTED: slot.record.state=RADIO_CMD_ACK_EXECUTED; break;
        case RADIO_ACK_DUPLICATE: slot.record.state=RADIO_CMD_ACK_DUPLICATE; break;
        default: slot.record.state=RADIO_CMD_ACK_REJECTED; break;
    }
    return true;
}
void RadioAckTracker::update(uint32_t now) {
    for(uint8_t i=0;i<CAPACITY;i++) {
        Slot &slot=slots_[i];
        if(slot.used && (slot.record.state==RADIO_CMD_ACK_WAITING || slot.record.state==RADIO_CMD_ACK_ACCEPTED) &&
           (uint32_t)(now-slot.record.createdAtMs)>=slot.timeoutMs) {
            slot.record.state=RADIO_CMD_ACK_TIMEOUT; slot.record.updatedAtMs=now;
        }
    }
}
bool RadioAckTracker::get(uint16_t id,RadioCommandAckRecord &out) const {
    const int index=find(id); if(index<0) return false; out=slots_[index].record; return true;
}
uint8_t RadioAckTracker::pendingCount() const {
    uint8_t count=0;
    for(uint8_t i=0;i<CAPACITY;i++) if(slots_[i].used &&
       (slots_[i].record.state==RADIO_CMD_ACK_WAITING || slots_[i].record.state==RADIO_CMD_ACK_ACCEPTED)) count++;
    return count;
}
