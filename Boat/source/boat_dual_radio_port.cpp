#include "boat_dual_radio_port.h"
#include "radio_payload_codec.h"
#include <string.h>


static constexpr uint8_t REMOTE_ID=0x01,BOAT_ID=0x02;

BoatDualRadioPort::BoatDualRadioPort()
 : nrf_(BOAT_NRF_CE_PIN,BOAT_NRF_CSN_PIN,&SPI,BOAT_NRF_CHANNEL,BOAT_NRF_SCK_PIN,BOAT_NRF_MISO_PIN,BOAT_NRF_MOSI_PIN),
   e22_(e22Serial_),nrfReady_(false),sx1262Ready_(false),sessionEpoch_(1),
   lastNrfRxMs_(0),lastBackupRxMs_(0),lastNrfHeartbeatTxMs_(0),
   lastBackupHeartbeatTxMs_(0),lastBackupHeartbeatAttemptMs_(0),txSequence_(0),authenticated_(false),
   sessionStarted_(false),lastNrfRxValid_(false),lastBackupRxValid_(false) {

 memset(key_,0,sizeof(key_));memset(rootKey_,0,sizeof(rootKey_));
}
void BoatDualRadioPort::abortMission(){
 controller_.emergencyAbort();stickLease_.reset();missionIntent_={INTENT_STOP,-1,0};
}
bool BoatDualRadioPort::missionInputs(ExecInputs &in,uint32_t now)const{
 in={};if(!missionInput_||!missionInput_(in))return false;
 in.primaryFresh=primaryFresh(now);in.backupFresh=backupFresh(now);return true;
}
bool BoatDualRadioPort::missionOverrideOutput(int16_t &left,int16_t &right)const{
 ExecInputs in{};const uint32_t now=millis();const bool ready=missionInputs(in,now);
 return stickLease_.output(controller_.status().transaction,now,ready&&controller_.temporaryOverrideConfirmed()&&in.primaryFresh&&!in.stopActive&&in.releaseIdle&&in.releaseInterlocks,left,right);
}
bool BoatDualRadioPort::missionReleaseComplete(uint16_t transaction,uint16_t token,bool success){
 ExecInputs in{};const bool ready=missionInputs(in,millis());
 const bool safe=ready&&in.primaryFresh&&!in.stopActive&&in.releaseInterlocks&&in.releaseIdle&&in.stationary;
 return controller_.releaseComplete(transaction,token,success&&safe,safe);
}
bool BoatDualRadioPort::sendHardwareTelemetry(const RadioDecodedPayload &status){
 if(!sessionReady()||!primaryFresh(millis()))return false;
 RadioFrame f;radio_frameInit(f,RADIO_MSG_TELEMETRY,BOAT_ID,REMOTE_ID,++txSequence_,0,millis(),sessionEpoch_);
 return radio_encodeTelemetryPayload(status,f.payload)&&radio_frameSeal(f,key_)&&sendFrame(LINK_NRF_PRIMARY,f);
}
bool BoatDualRadioPort::sendMissionFeedback(const MissionControlFeedback &status,uint32_t now){
 if(!sessionReady()||!nrfReady_)return false;
 RadioFrame f;radio_frameInit(f,RADIO_MSG_MISSION_FEEDBACK,BOAT_ID,REMOTE_ID,++txSequence_,status.transaction,now,sessionEpoch_);
 return missionFeedbackEncode(status,f.payload)&&radio_frameSeal(f,key_)&&sendFrame(LINK_NRF_PRIMARY,f);
}
void BoatDualRadioPort::resetSession(bool preserveReturn){
 ++generation_;releaseIdSeen_=false;
 ExecInputs in{};const bool ready=preserveReturn&&missionInputs(in,millis());
 detachedReturn_=ready&&controller_.divertHome(in,true);
 stickLease_.reset();missionIntent_={INTENT_STOP,-1,0};
 if(!detachedReturn_){abortMission();controller_.resetSession();}
 handshake_.reset();radioSession_wipe(key_,32);sessionStarted_=false;
 primarySeqGuard_.resetAfterHandshake();backupSeqGuard_.resetAfterHandshake();crossRadioSeqGuard_.resetAfterHandshake();
 lastNrfRxValid_=lastBackupRxValid_=false;commandQueue_.clear();mission_.reset();dedupe_.clear();txSequence_=0;
}
bool BoatDualRadioPort::begin() {
 resetSession();nrfReady_=false;sx1262Ready_=false;authenticated_=radioKey_get(rootKey_);
 lastNrfHeartbeatTxMs_=lastBackupHeartbeatTxMs_=lastBackupHeartbeatAttemptMs_=0;
// The external E22 UART and nRF SPI use separate peripherals.
// Every pin remains -1 until an audited hardware profile is supplied.
#if BOAT_NRF_PIN_AUDIT_SIGNED && BOAT_NRF_CE_PIN >= 0 && BOAT_NRF_CSN_PIN >= 0 && BOAT_NRF_SCK_PIN >= 0 && BOAT_NRF_MISO_PIN >= 0 && BOAT_NRF_MOSI_PIN >= 0
 nrfReady_=nrf_.begin(false);
#endif
#if BOAT_E22_UART_PIN_TX >= 0 && BOAT_E22_UART_PIN_RX >= 0 && BOAT_E22_UART_PIN_M0 >= 0 && BOAT_E22_UART_PIN_M1 >= 0 && BOAT_E22_UART_PIN_AUX >= 0 && BOAT_E22_UART_RF_INTEROPERABILITY_VERIFIED && BOAT_E22_UART_LOGIC_LEVELS_VERIFIED && BOAT_E22_UART_PIN_AUDIT_SIGNED
 const E22BoardPins pins={(int8_t)BOAT_E22_UART_PIN_TX,(int8_t)BOAT_E22_UART_PIN_RX,
  (int8_t)BOAT_E22_UART_PIN_M0,(int8_t)BOAT_E22_UART_PIN_M1,(int8_t)BOAT_E22_UART_PIN_AUX,
  true,true,true};
 sx1262Ready_=e22_.begin(pins);
#endif

 // Wait for the remote's authenticated session-start marker. Its epoch is the
 // session identifier used by both endpoints; do not invent a boat-local epoch.
 return nrfReady_||sx1262Ready_;
}
void BoatDualRadioPort::update() {
 const uint32_t now=millis();
 if(sessionStarted_&&(!sessionReady()||now-lastSessionRx_>=6500U))resetSession(true);
 if(handshake_.timeout(now))resetSession(detachedReturn_);
 if(!missionGate_||!missionGate_()||!primaryFresh(now))mission_.reset();
 if(nrfReady_){nrf_.update();RadioFrame f;while(nrf_.receiveFrame(f))receive(LINK_NRF_PRIMARY,f);}
 if(sx1262Ready_){e22_.update();RadioFrame f;while(e22_.receiveFrame(f))receive(LINK_LORA_BACKUP,f);}
 const auto progress=controller_.status();
 if(progress.transaction){
   ExecInputs in{};const bool ready=missionInputs(in,now);
   missionIntent_=controller_.tick(in,now,ready,stickLease_.neutral(progress.transaction,now));
   int16_t left=0,right=0;
   if(controller_.temporaryOverrideConfirmed()&&!missionOverrideOutput(left,right))abortMission();
   if(detachedReturn_&&!controller_.diverted()){detachedReturn_=false;controller_.resetSession();missionIntent_={INTENT_STOP,-1,0};}
   if(!detachedReturn_&&controller_.status().transaction&&now-lastMissionStatusMs_>=250U){lastMissionStatusMs_=now;sendMissionFeedback(controller_.status(),now);}
 }
 if(sessionReady()) {
   if(nrfReady_ && (uint32_t)(now-lastNrfHeartbeatTxMs_)>=100U) {lastNrfHeartbeatTxMs_=now;sendHeartbeat(LINK_NRF_PRIMARY,now);}
   if(sx1262Ready_ && (uint32_t)(now-lastBackupHeartbeatTxMs_)>=3000U && (uint32_t)(now-lastBackupHeartbeatAttemptMs_)>=250U) {lastBackupHeartbeatAttemptMs_=now;if(sendHeartbeat(LINK_LORA_BACKUP,now))lastBackupHeartbeatTxMs_=now;}
 }
}
void BoatDualRadioPort::receive(ManagedLink link,const RadioFrame &f) {
 const uint32_t now=millis();
 if(!authenticated_||!radioLinkFlagMatches(f.flags,link==LINK_LORA_BACKUP)||f.sourceId!=REMOTE_ID||f.destinationId!=BOAT_ID)return;
 if(f.type>=SESSION_HELLO&&f.type<=SESSION_CONFIRM){
   if(link!=LINK_NRF_PRIMARY||f.sequence||f.commandId||f.sessionEpoch||!radio_frameVerify(f,rootKey_))return;
   SessionHandshakePacket reply{};bool newSession=false;
   if(!handshake_.boatReceive((SessionHandshakeMessage)f.type,f.payload,radioPlatformNonce(),rootKey_,radioPlatformHmac,now,true,reply,newSession))return;
   if(newSession){
     ++generation_;releaseIdSeen_=false;
     handshake_.frameKey(key_);sessionStarted_=true;lastSessionRx_=now;txSequence_=0;
     primarySeqGuard_.resetAfterHandshake();backupSeqGuard_.resetAfterHandshake();crossRadioSeqGuard_.resetAfterHandshake();
     lastNrfRxValid_=lastBackupRxValid_=false;commandQueue_.clear();mission_.reset();dedupe_.clear();stickLease_.reset();if(!detachedReturn_){abortMission();controller_.resetSession();}
   }
   RadioFrame ack;radio_frameInit(ack,(RadioMessageType)reply.type,BOAT_ID,REMOTE_ID,0,0,now,0);
   memcpy(ack.payload,reply.payload,8);if(radio_frameSeal(ack,rootKey_))nrf_.sendFrame(ack);
   return;
 }
 if(!sessionStarted_||f.sessionEpoch!=sessionEpoch_||!radio_frameVerify(f,key_))return;
 if(f.type!=RADIO_MSG_HEARTBEAT&&f.type!=RADIO_MSG_CONTROL&&f.type!=RADIO_MSG_ACTION&&f.type!=RADIO_MSG_MISSION_PREPARE&&f.type!=RADIO_MSG_MISSION_CONTROL&&f.type!=RADIO_MSG_MISSION_STICKS)return;
 RadioSessionCounter &linkGuard=link==LINK_NRF_PRIMARY?primarySeqGuard_:backupSeqGuard_;
 if(!linkGuard.accept(f.sequence))return;
 const bool crossSequenceAccepted=crossRadioSeqGuard_.accept(f.sequence);
 if(f.type==RADIO_MSG_HEARTBEAT){
   RadioDecodedPayload heartbeat{};if(!radio_decodeHeartbeatPayload(f.payload,heartbeat))return;
 }else if(!crossSequenceAccepted&&!(f.type==RADIO_MSG_ACTION&&f.payload[0]==CMD_EMERGENCY_STOP))return;
 bool &rxValid=link==LINK_NRF_PRIMARY?lastNrfRxValid_:lastBackupRxValid_;
 uint32_t &lastRx=link==LINK_NRF_PRIMARY?lastNrfRxMs_:lastBackupRxMs_;
 if(f.type==RADIO_MSG_HEARTBEAT){lastRx=lastSessionRx_=now;rxValid=true;return;}
 // Detached Home return keeps old plan private to the local supervisor.
 // A new session cannot attach to the old transaction or restart its target.
 if(detachedReturn_&&(f.type==RADIO_MSG_MISSION_STICKS||f.type==RADIO_MSG_MISSION_CONTROL||f.type==RADIO_MSG_MISSION_PREPARE))return;
 if(f.type==RADIO_MSG_MISSION_STICKS){
   if(link!=LINK_NRF_PRIMARY||!crossSequenceAccepted)return;
   MissionStickInput input{};if(!missionStickDecode(f.payload,input)||f.commandId!=input.transaction)return;
   if(stickLease_.receive(input,controller_.status().transaction,now,true)){lastRx=lastSessionRx_=now;rxValid=true;}
   return;
 }
 if(f.type==RADIO_MSG_MISSION_CONTROL){
   if(link!=LINK_NRF_PRIMARY||!crossSequenceAccepted)return;
   MissionControlRequest request{};if(!missionControlDecode(f.payload,request)||f.commandId!=request.transaction)return;
   lastRx=lastSessionRx_=now;rxValid=true;
   ExecInputs in{};const bool ready=missionInputs(in,now);
   MissionControlFeedback ack{};const bool newRequest=request.request>controller_.lastRequestId();
   if(controller_.request(request,in,now,true,ready,stickLease_.neutral(request.transaction,now),ack)){
     if(newRequest&&ack.result==MC_ACCEPTED&&request.op==MCTRL_START)commandQueue_.clear();
     if(newRequest&&ack.result==MC_ACCEPTED&&request.op!=MCTRL_START&&request.op!=MCTRL_RESUME)missionIntent_={INTENT_STOP,-1,0};
     if(newRequest&&ack.result==MC_ACCEPTED&&(request.op==MCTRL_TEMP_OVERRIDE||request.op==MCTRL_CANCEL||request.op==MCTRL_SWITCH_MANUAL))stickLease_.reset();
     sendMissionFeedback(ack,now);
   }else sendMissionFeedback({request.transaction,MC_IDLE,(uint8_t)request.op,request.request,MC_REJECTED},now);
   return;
 }
 if(f.type==RADIO_MSG_MISSION_PREPARE){
   if(link!=LINK_NRF_PRIMARY||!crossSequenceAccepted||!sessionReady())return;
   const uint16_t id=(uint16_t)f.payload[1]|((uint16_t)f.payload[2]<<8);
   const MissionOpcode op=(MissionOpcode)f.payload[0];
   if(!id||id!=f.commandId||op<MISSION_BEGIN||op>MISSION_CANCEL)return;
   lastRx=lastSessionRx_=now;rxValid=true;
   const bool stopped=missionGate_&&missionGate_();
   MissionReceive result=mission_.ingest(f.payload,now,true,stopped);
   if(result==MISSION_PREPARED){MissionPlan plan{};if(!mission_.getPrepared(plan,now)||!controller_.stage(plan,now,true,stopped)){mission_.reset();result=MISSION_REJECTED;}else {stickLease_.reset();commandQueue_.clear();}}
   if(result==MISSION_CANCELLED)abortMission();
   RadioFrame ack;radio_frameInit(ack,RADIO_MSG_MISSION_ACK,BOAT_ID,REMOTE_ID,++txSequence_,id,now,sessionEpoch_);
   if(missionAck_encode(id,op,result,ack.payload)&&radio_frameSeal(ack,key_))sendFrame(link,ack);
   return;
 }
 if(f.type==RADIO_MSG_HEARTBEAT || f.type==RADIO_MSG_TELEMETRY || f.type==RADIO_MSG_ACK || f.type==RADIO_MSG_LINK_PROBE) return;
 BoatCommand cmd;
 if(!radio_decodeCommand(f,REMOTE_ID,BOAT_ID,now,2500,link==LINK_LORA_BACKUP,cmd,key_)) return;
 const uint16_t id=(uint16_t)cmd.commandId;
 if(id==0) return;
 // An authenticated STOP invalidates any prepared plan before queue handling.
 lastRx=lastSessionRx_=now;rxValid=true;
 if(cmd.type==CMD_EMERGENCY_STOP){const bool wasDetached=detachedReturn_;detachedReturn_=false;abortMission();if(wasDetached)controller_.resetSession();mission_.reset();commandQueue_.clear();enqueue(cmd,link,false);if(sessionReady())sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);return;}
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
 const auto phase=controller_.status().phase;
 if(detachedReturn_||(phase>=MC_PREPARED&&phase<=MC_RETURNING&&!(phase==MC_PREPARED&&cmd.type==CMD_CLEAR_STOP))){if(cmd.type!=CMD_MANUAL_MOTORS)sendCommandAck(link,cmd,RADIO_ACK_REJECTED_SAFETY,2,0,now);return;}
 if(dedupe_.hasExecuted(f.sourceId,f.sessionEpoch,id)) {
   sendCommandAck(link,cmd,RADIO_ACK_DUPLICATE,0,0,now);return;
 }
 if(commandPending(f.sessionEpoch,id)) {
   if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
   return;
 }
 const bool release=cmd.type>=CMD_RELEASE_HOPPER_LEFT&&cmd.type<=CMD_RELEASE_HOOK_RIGHT;
 if(release&&releaseIdSeen_){const uint16_t delta=(uint16_t)(id-lastReleaseId_);if(!delta||delta>=0x8000U){sendCommandAck(link,cmd,RADIO_ACK_REJECTED_SAFETY,2,0,now);return;}}
 if(!enqueue(cmd,link,false)) {
   if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_BUSY,0,0,now);
   return;
 }
 if(release){lastReleaseId_=id;releaseIdSeen_=true;}
 // ACCEPTED means only queued for application review. It is not EXECUTED.
 if(cmd.type!=CMD_MANUAL_MOTORS) sendCommandAck(link,cmd,RADIO_ACK_ACCEPTED,0,0,now);
}
bool BoatDualRadioPort::commandPending(uint8_t epoch,uint16_t commandId) const {
 return commandQueue_.contains(epoch,commandId);
}
bool BoatDualRadioPort::sendCommandAck(ManagedLink link,const BoatCommand &cmd,uint8_t result,uint8_t stopState,uint8_t interlockFlags,uint32_t now) {
 if(!sessionReady())return false;
 RadioFrame f;
 radio_frameInit(f,RADIO_MSG_ACK,BOAT_ID,REMOTE_ID,++txSequence_,(uint16_t)cmd.commandId,now,sessionEpoch_);
 if(!radio_encodeAckPayload(result,(uint8_t)cmd.type,(uint16_t)cmd.commandId,stopState,interlockFlags,f.payload))return false;
 if(!radio_frameSeal(f,key_))return false;
 return sendFrame(link,f);
}
bool BoatDualRadioPort::enqueue(const BoatCommand &cmd,ManagedLink link,bool duplicate) {
 return commandQueue_.push(cmd,(uint8_t)link,duplicate,sessionEpoch_);
}
bool BoatDualRadioPort::takeCommand(BoatCommand &cmd,bool &duplicate,ManagedLink &link) {
 uint8_t linkId=0,epoch=0;
 while(commandQueue_.pop(cmd,duplicate,linkId,epoch)){
    if(!sessionStarted_||epoch!=sessionEpoch_)continue;
    if(cmd.type!=CMD_EMERGENCY_STOP&&millis()-cmd.createdAtMs>(cmd.type==CMD_MANUAL_MOTORS?250U:1500U))continue;
    link=(ManagedLink)linkId;return true;
 }
 return false;
}
bool BoatDualRadioPort::completeApplicationAction(const BoatCommand &cmd,uint32_t generation,bool success,uint8_t stopState,uint8_t interlocks){
 if(generation!=generation_||!sessionReady())return false;
 if(success)dedupe_.markExecuted(REMOTE_ID,sessionEpoch_,(uint16_t)cmd.commandId);
 return sendCommandAck(LINK_NRF_PRIMARY,cmd,success?RADIO_ACK_EXECUTED:RADIO_ACK_REJECTED_SAFETY,stopState,interlocks,millis());
}
void BoatDualRadioPort::markActionExecuted(uint8_t sender,uint8_t epoch,uint16_t commandId) {
 dedupe_.markExecuted(sender,epoch,commandId);
}
bool BoatDualRadioPort::sendFrame(ManagedLink link,const RadioFrame &f) {
 if(!sessionStarted_||!radio_frameVerify(f,key_)||!f.sequence||f.sequence>=RadioSessionCounter::REKEY_AT)return false;
 if(link==LINK_NRF_PRIMARY && nrfReady_)return nrf_.sendFrame(f);
 if(link==LINK_LORA_BACKUP && sx1262Ready_){RadioFrame backup=f;backup.flags=RADIO_FLAG_BACKUP_LINK;if(!radio_frameSeal(backup,key_))return false;return e22_.sendFrame(backup,millis());}
 return false;
}
bool BoatDualRadioPort::sendHeartbeat(ManagedLink link,uint32_t now) {
 if(!sessionReady())return false;
 RadioFrame f;radio_frameInit(f,RADIO_MSG_HEARTBEAT,BOAT_ID,REMOTE_ID,++txSequence_,0,now,sessionEpoch_);
 f.payload[0]=(uint8_t)(1U|(nrfReady_?2U:0U)|(sx1262Ready_?4U:0U)); // alive + radio availability; STOP state unknown
 f.payload[1]=1; // protocol build-major placeholder; reserved payload bytes remain zero
 if(!radio_frameSeal(f,key_))return false;
 return sendFrame(link,f);
}
bool BoatDualRadioPort::primaryFresh(uint32_t now)const{return sessionStarted_ && nrfReady_ && lastNrfRxValid_ && (uint32_t)(now-lastNrfRxMs_)<=250;}
bool BoatDualRadioPort::backupFresh(uint32_t now)const{return sessionStarted_ && sx1262Ready_ && lastBackupRxValid_ && (uint32_t)(now-lastBackupRxMs_)<=6000;}
BoatFailsafeDecision BoatDualRadioPort::evaluateFailsafe(uint32_t now,bool mission,bool gnss,bool heading,bool battery,bool geofence)const {
 BoatLinkSafetyInputs in={primaryFresh(now),backupFresh(now),mission,gnss,heading,battery,geofence};
 return boatEvaluateLinkFailsafe(in);
}

