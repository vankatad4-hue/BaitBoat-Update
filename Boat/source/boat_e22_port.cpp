#include "boat_e22_port.h"
#include <string.h>

BoatE22Port::BoatE22Port()
 : serial_(1),link_(serial_,modePins,auxReady,resetPin,this),ready_(false),
   rxAvailable_(false),duplicateAction_(false) { memset(key_,0,sizeof(key_)); }

void BoatE22Port::modePins(bool m0,bool m1,void*) {
#if BOAT_E22_M0_PIN >= 0 && BOAT_E22_M1_PIN >= 0
    digitalWrite(BOAT_E22_M0_PIN,m0?HIGH:LOW);
    digitalWrite(BOAT_E22_M1_PIN,m1?HIGH:LOW);
#else
    (void)m0;(void)m1;
#endif
}
bool BoatE22Port::auxReady(void*) {
#if BOAT_E22_AUX_PIN >= 0
    return digitalRead(BOAT_E22_AUX_PIN)==HIGH;
#else
    return false;
#endif
}
void BoatE22Port::resetPin(bool released,void*) {
#if BOAT_E22_RESET_PIN >= 0
    digitalWrite(BOAT_E22_RESET_PIN,released?HIGH:LOW);
#else
    (void)released;
#endif
}
bool BoatE22Port::begin() {
#if BOAT_E22_UART_RX_PIN >= 0 && BOAT_E22_UART_TX_PIN >= 0 && BOAT_E22_M0_PIN >= 0 && BOAT_E22_M1_PIN >= 0 && BOAT_E22_AUX_PIN >= 0
    pinMode(BOAT_E22_M0_PIN,OUTPUT);pinMode(BOAT_E22_M1_PIN,OUTPUT);
    pinMode(BOAT_E22_AUX_PIN,INPUT);
#if BOAT_E22_RESET_PIN >= 0
    pinMode(BOAT_E22_RESET_PIN,OUTPUT);
    digitalWrite(BOAT_E22_RESET_PIN,HIGH);
#endif
    modePins(false,false,this);
    serial_.begin(9600,SERIAL_8N1,BOAT_E22_UART_RX_PIN,BOAT_E22_UART_TX_PIN);
    ready_=link_.configureEU868TestProfile();
    if(ready_) ready_=radioKey_get(key_); // no key => transport is receive-only and fails closed
    return ready_;
#else
    // Intentionally no default GPIO values. Prevent accidental UART/radio init.
    ready_=false;return false;
#endif
}
void BoatE22Port::update() {
    if(!link_.ready()) return;
    link_.update();
    RadioFrame f;
    while(link_.receiveFrame(f)) {
        if(!radioKey_get(key_)) continue;
        BoatCommand command;
        if(!radio_decodeCommand(f,0x01,0x02,millis(),2500,true,command,key_)) continue;
        if(!sequenceGuard_.accept(f.sessionEpoch,f.sequence)) continue;
        const bool duplicate=dedupe_.hasExecuted(f.sourceId,f.sessionEpoch,f.commandId);
        decoded_=command;duplicateAction_=duplicate;rxAvailable_=true;rxFrame_=f;
    }
}
bool BoatE22Port::takeCommand(BoatCommand &command,bool &duplicateAction) {
    if(!rxAvailable_) return false;
    command=decoded_;duplicateAction=duplicateAction_;rxAvailable_=false;return true;
}
void BoatE22Port::markActionExecuted(uint8_t source,uint8_t epoch,uint16_t commandId) {
    dedupe_.markExecuted(source,epoch,commandId);
}
