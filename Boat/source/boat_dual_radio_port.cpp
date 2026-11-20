#include "boat_dual_radio_port.h"
#include <string.h>

static constexpr uint8_t REMOTE_ID=0x01,BOAT_ID=0x02;
static constexpr uint8_t BOAT_SESSION_EPOCH=1;

BoatDualRadioPort::BoatDualRadioPort()
 : serial_(1),nrf_(BOAT_NRF_CE_PIN,BOAT_NRF_CSN_PIN,&SPI,76),
   e22_(serial_,
      [](bool m0,bool m1,void*){if(BOAT_E22_M0_PIN>=0)digitalWrite(BOAT_E22_M0_PIN,m0?HIGH:LOW);if(BOAT_E22_M1_PIN>=0)digitalWrite(BOAT_E22_M1_PIN,m1?HIGH:LOW);},
      [](void*){return BOAT_E22_AUX_PIN>=0 && digitalRead(BOAT_E22_AUX_PIN)==HIGH;},
      [](bool released,void*){if(BOAT_E22_RESET_PIN>=0)digitalWrite(BOAT_E22_RESET_PIN,released?HIGH:LOW);},nullptr),
   nrfReady_(false),e22Configured_(false),qWrite_(0),qRead_(0),qCount_(0),
   lastNrfRxMs_(0),lastE22RxMs_(0),authenticated_(false) {
 memset(key_,0,sizeof(key_));memset(queue_,0,sizeof(queue_));
}
bool BoatDualRadioPort::begin() {
 nrfReady_=false;e22Configured_=false;authenticated_=radioKey_get(key_);
#if BOAT_NRF_CE_PIN >= 0 && BOAT_NRF_CSN_PIN >= 0
 nrfReady_=nrf_.begin(false);
#endif
#if BOAT_E22_UART_RX_PIN >= 0 && BOAT_E22_UART_TX_PIN >= 0 && BOAT_E22_M0_PIN >= 0 && BOAT_E22_M1_PIN >= 0 && BOAT_E22_AUX_PIN >= 0
 pinMode(BOAT_E22_M0_PIN,OUTPUT);pinMode(BOAT_E22_M1_PIN,OUTPUT);pinMode(BOAT_E22_AUX_PIN,INPUT);
 #if BOAT_E22_RESET_PIN >= 0
 pinMode(BOAT_E22_RESET_PIN,OUTPUT);digitalWrite(BOAT_E22_RESET_PIN,HIGH);
 #endif
 serial_.begin(9600,SERIAL_8N1,BOAT_E22_UART_RX_PIN,BOAT_E22_UART_TX_PIN);
 e22Configured_=e22_.configureEU868TestProfile();
#endif
 return nrfReady_||e22Configured_;
}
void BoatDualRadioPort::update() {
 if(nrfReady_){nrf_.update();RadioFrame f;while(nrf_.receiveFrame(f))receive(LINK_NRF_PRIMARY,f);}
 if(e22Configured_){e22_.update();RadioFrame f;while(e22_.receiveFrame(f))receive(LINK_LORA_BACKUP,f);}
}
void BoatDualRadioPort::receive(ManagedLink link,const RadioFrame &f) {
 const uint32_t now=millis();
 if(!authenticated_ || !radio_frameVerify(f,key_) || f.sourceId!=REMOTE_ID || f.destinationId!=BOAT_ID) return;
 if(!seqGuard_.accept(f.sessionEpoch,f.sequence)) return;
 if(link==LINK_NRF_PRIMARY)lastNrfRxMs_=now;else lastE22RxMs_=now;
 if(f.type==RADIO_MSG_HEARTBEAT || f.type==RADIO_MSG_TELEMETRY || f.type==RADIO_MSG_ACK || f.type==RADIO_MSG_LINK_PROBE)
     return;
 BoatCommand cmd;
 if(!radio_decodeCommand(f,REMOTE_ID,BOAT_ID,now,2500,link==LINK_LORA_BACKUP,cmd,key_)) return;
 const bool duplicate=dedupe_.hasExecuted(f.sourceId,f.sessionEpoch,f.commandId);
 enqueue(cmd,link,duplicate);
}
bool BoatDualRadioPort::enqueue(const BoatCommand &cmd,ManagedLink link,bool duplicate) {
 if(qCount_>=4)return false;
 queue_[qWrite_]={cmd,link,duplicate};
 qWrite_=(uint8_t)((qWrite_+1U)%4U);++qCount_;return true;
}
bool BoatDualRadioPort::takeCommand(BoatCommand &cmd,bool &duplicate,ManagedLink &link) {
 if(qCount_==0)return false;
 const QueuedCommand q=queue_[qRead_];qRead_=(uint8_t)((qRead_+1U)%4U);--qCount_;
 cmd=q.cmd;duplicate=q.duplicate;link=q.link;return true;
}
void BoatDualRadioPort::markActionExecuted(uint8_t sender,uint8_t epoch,uint16_t commandId) {
 dedupe_.markExecuted(sender,epoch,commandId);
}
bool BoatDualRadioPort::sendFrame(ManagedLink link,const RadioFrame &f) {
 if(!authenticated_)return false;
 if(link==LINK_NRF_PRIMARY && nrfReady_)return nrf_.sendFrame(f);
 if(link==LINK_LORA_BACKUP && e22Configured_)return e22_.sendFrame(f,millis());
 return false;
}
bool BoatDualRadioPort::sendHeartbeat(ManagedLink link,uint16_t seq,uint32_t now) {
 if(!authenticated_)return false;
 RadioFrame f;radio_frameInit(f,RADIO_MSG_HEARTBEAT,BOAT_ID,REMOTE_ID,seq,0,now,BOAT_SESSION_EPOCH);
 f.payload[0]=nrfReady_?1:0;f.payload[1]=e22Configured_?1:0;
 f.payload[2]=0;f.payload[3]=0; // boat state fields to be assigned by application
 if(!radio_frameSeal(f,key_))return false;
 return sendFrame(link,f);
}
bool BoatDualRadioPort::primaryFresh(uint32_t now)const{return nrfReady_ && lastNrfRxMs_ && (uint32_t)(now-lastNrfRxMs_)<=250;}
bool BoatDualRadioPort::backupFresh(uint32_t now)const{return e22Configured_ && lastE22RxMs_ && (uint32_t)(now-lastE22RxMs_)<=6000;}
BoatFailsafeDecision BoatDualRadioPort::evaluateFailsafe(uint32_t now,bool mission,bool gnss,bool heading,bool battery,bool geofence)const {
 BoatLinkSafetyInputs in={primaryFresh(now),backupFresh(now),mission,gnss,heading,battery,geofence};
 return boatEvaluateLinkFailsafe(in);
}
