#include "boat_dual_radio_port.h"
#include <string.h>


static constexpr uint8_t REMOTE_ID=0x01,BOAT_ID=0x02;

BoatDualRadioPort::BoatDualRadioPort()
 : serial_(1),nrf_(BOAT_NRF_CE_PIN,BOAT_NRF_CSN_PIN,&SPI,76),
   e22_(serial_,
      [](bool m0,bool m1,void*){if(BOAT_E22_M0_PIN>=0)digitalWrite(BOAT_E22_M0_PIN,m0?HIGH:LOW);if(BOAT_E22_M1_PIN>=0)digitalWrite(BOAT_E22_M1_PIN,m1?HIGH:LOW);},
      [](void*){return BOAT_E22_AUX_PIN>=0 && digitalRead(BOAT_E22_AUX_PIN)==HIGH;},
      [](bool released,void*){if(BOAT_E22_RESET_PIN>=0)digitalWrite(BOAT_E22_RESET_PIN,released?HIGH:LOW);},nullptr),
   nrfReady_(false),e22Configured_(false),sessionEpoch_(0),sessionStartSeenPrimary_(false),sessionStartSeenBackup_(false),
   lastNrfRxMs_(0),lastE22RxMs_(0),lastNrfHeartbeatTxMs_(0),
   lastE22HeartbeatTxMs_(0),lastE22HeartbeatAttemptMs_(0),txSequence_(0),authenticated_(false),
   sessionStarted_(false),lastNrfRxValid_(false),lastE22RxValid_(false) {

 memset(key_,0,sizeof(key_));
}
bool BoatDualRadioPort::begin() {
 nrfReady_=false;e22Configured_=false;authenticated_=radioKey_get(key_);sessionStarted_=false;primarySeqGuard_.reset();backupSeqGuard_.reset();crossRadioSeqGuard_.reset();sessionStartSeenPrimary_=false;sessionStartSeenBackup_=false;lastNrfRxValid_=false;lastE22RxValid_=false;commandQueue_.clear();txSequence_=0;lastNrfHeartbeatTxMs_=0;lastE22HeartbeatTxMs_=0;lastE22HeartbeatAttemptMs_=0;
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
 // Wait for the remote's authenticated session-start marker. Its epoch is the
 // session identifier used by both endpoints; do not invent a boat-local epoch.
 return nrfReady_||e22Configured_;
}
void BoatDualRadioPort::update() {
 const uint32_t now=millis();
 if(nrfReady_){nrf_.update();RadioFrame f;while(nrf_.receiveFrame(f))receive(LINK_NRF_PRIMARY,f);}
 if(e22Configured_){e22_.update();RadioFrame f;while(e22_.receiveFrame(f))receive(LINK_LORA_BACKUP,f);}
 if(authenticated_ && sessionStarted_) {
   if(nrfReady_ && (uint32_t)(now-lastNrfHeartbeatTxMs_)>=100U) {lastNrfHeartbeatTxMs_=now;sendHeartbeat(LINK_NRF_PRIMARY,now);}
   if(e22Configured_ && (uint32_t)(now-lastE22HeartbeatTxMs_)>=3000U && (uint32_t)(now-lastE22HeartbeatAttemptMs_)>=250U) {lastE22HeartbeatAttemptMs_=now;if(sendHeartbeat(LINK_LORA_BACKUP,now))lastE22HeartbeatTxMs_=now;}
 }
}
void BoatDualRadioPort::receive(ManagedLink link,const RadioFrame &f) {
 const uint32_t now=millis();
 if(!authenticated_ || !radio_frameVerify(f,key_) || f.flags!=0 || f.sourceId!=REMOTE_ID || f.destinationId!=BOAT_ID) return;
 RadioSequenceGuard &linkGuard=(link==LINK_NRF_PRIMARY)?primarySeqGuard_:backupSeqGuard_;
 bool &linkSessionSeen=(link==LINK_NRF_PRIMARY)?sessionStartSeenPrimary_:sessionStartSeenBackup_;
 bool &rxValid=(link==LINK_NRF_PRIMARY)?lastNrfRxValid_:lastE22RxValid_;
 uint32_t &lastRx=(link==LINK_NRF_PRIMARY)?lastNrfRxMs_:lastE22RxMs_;
 if(f.type==RADIO_MSG_SESSION_START) {
   if(f.sequence!=0 || f.commandId!=0 || f.payload[0]!=f.sessionEpoch) return;
   for(uint8_t i=1;i<8;i++) if(f.payload[i]!=0) return;
   if(sessionStarted_ && f.sessionEpoch==sessionEpoch_) {
     if(linkSessionSeen) return;
     if(!linkGuard.accept(f.sessionEpoch,0,true)) return;
     linkSessionSeen=true;lastRx=now;rxValid=true;
     // Reply using the remote's session epoch so both radios agree on it.
     sendSessionStart(link,now);
     return;
   }
   // A new authenticated epoch starts a fresh receive session. Queue contents
   // are discarded so commands from the previous session cannot execute.
   sessionEpoch_=f.sessionEpoch;sessionStarted_=true;
   primarySeqGuard_.reset();backupSeqGuard_.reset();crossRadioSeqGuard_.reset();
   sessionStartSeenPrimary_=false;sessionStartSeenBackup_=false;
   lastNrfRxValid_=false;lastE22RxValid_=false;
   commandQueue_.clear();txSequence_=0;
   if(!linkGuard.accept(f.sessionEpoch,0,true)) return;
   linkSessionSeen=true;lastRx=now;rxValid=true;
   sendSessionStart(LINK_NRF_PRIMARY,now);
   if(e22Configured_) sendSessionStart(LINK_LORA_BACKUP,now);
   return;
 }
 if(!sessionStarted_ || f.sessionEpoch!=sessionEpoch_ || !linkGuard.accept(f.sessionEpoch,f.sequence,false)) return;
 lastRx=now;rxValid=true;
 const bool isCommand=(f.type==RADIO_MSG_CONTROL || f.type==RADIO_MSG_ACTION);
 const bool crossSequenceAccepted=crossRadioSeqGuard_.accept(f.sessionEpoch,f.sequence,false);
 if(!crossSequenceAccepted && !isCommand) return;
 if(f.type==RADIO_MSG_HEARTBEAT || f.type==RADIO_MSG_TELEMETRY || f.type==RADIO_MSG_ACK || f.type==RADIO_MSG_LINK_PROBE) return;
 BoatCommand cmd;
 if(!radio_decodeCommand(f,REMOTE_ID,BOAT_ID,now,2500,link==LINK_LORA_BACKUP,cmd,key_)) return;
 const uint16_t id=(uint16_t)cmd.commandId;
 if(id==0) return;
 // A command frame rejected by the cross-radio sequence guard is accepted
 // only as an exact command-ID duplicate. This prevents replaying an old
 // sequence with a new command ID on the alternate radio.
 if(!crossSequenceAccepted) {
   const BoatCrossRadioCommandDecision decision=boatClassifyCrossRadioCommand(
       false,dedupe_.hasExecuted(f.sourceId,f.sessionEpoch,id),commandPending(f.sessionEpoch,id));
   if(decision==BOAT_CROSSRADIO_DUPLICATE_EXECUTED)
     sendCommandAck(link,cmd,RADIO_ACK_DUPLICATE,0,0,now);
   else if(decision==BOAT_CROSSRADIO_DUPLICATE_PENDING && cmd.type!=CMD_MANUAL_MOTORS)
     sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
   return;
 }
 if(dedupe_.hasExecuted(f.sourceId,f.sessionEpoch,id)) {
   sendCommandAck(link,cmd,RADIO_ACK_DUPLICATE,0,0,now);return;
 }
 if(commandPending(f.sessionEpoch,id)) {
   if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
   return;
 }
 if(!enqueue(cmd,link,false)) {
   if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_BUSY,0,0,now);
   return;
 }
 // ACCEPTED means only queued for application review. It is not EXECUTED.
 if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
}
bool BoatDualRadioPort::enqueue(const BoatCommand &cmd,ManagedLink link,bool duplicate) {
 return commandQueue_.push(cmd,(uint8_t)link,duplicate,sessionEpoch_);
}
bool BoatDualRadioPort::takeCommand(BoatCommand &cmd,bool &duplicate,ManagedLink &link) {
 uint8_t linkId=0,epoch=0;
 if(!commandQueue_.pop(cmd,duplicate,linkId,epoch))return false;
 link=(ManagedLink)linkId;return true;
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
bool BoatDualRadioPort::sendHeartbeat(ManagedLink link,uint32_t now) {
 if(!authenticated_)return false;
 RadioFrame f;radio_frameInit(f,RADIO_MSG_HEARTBEAT,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
 f.payload[0]=(uint8_t)(1U|(nrfReady_?2U:0U)|(e22Configured_?4U:0U)); // alive + radio availability; STOP state unknown
 f.payload[1]=1; // protocol build-major placeholder; reserved payload bytes remain zero
 if(!radio_frameSeal(f,key_))return false;
 return sendFrame(link,f);
}
bool BoatDualRadioPort::primaryFresh(uint32_t now)const{return nrfReady_ && lastNrfRxValid_ && (uint32_t)(now-lastNrfRxMs_)<=250;}
bool BoatDualRadioPort::backupFresh(uint32_t now)const{return e22Configured_ && lastE22RxValid_ && (uint32_t)(now-lastE22RxMs_)<=6000;}
BoatFailsafeDecision BoatDualRadioPort::evaluateFailsafe(uint32_t now,bool mission,bool gnss,bool heading,bool battery,bool geofence)const {
 BoatLinkSafetyInputs in={primaryFresh(now),backupFresh(now),mission,gnss,heading,battery,geofence};
 return boatEvaluateLinkFailsafe(in);
}

bool BoatDualRadioPort::sendSessionStart(ManagedLink link,uint32_t now) {
 if(!authenticated_)return false;
 RadioFrame f;radio_frameInit(f,RADIO_MSG_SESSION_START,BOAT_ID,REMOTE_ID,0,0,now,sessionEpoch_);
 f.payload[0]=sessionEpoch_;
 if(!radio_frameSeal(f,key_))return false;
 return sendFrame(link,f);
}
