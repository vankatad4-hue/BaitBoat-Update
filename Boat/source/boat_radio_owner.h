#pragma once
// Consolidated Boat radio component; original types and owner contracts retained.
#include "boat_radio_application.h"
#include "recovery_probe_wire.h"
namespace flytec_r30h123 {
using namespace flytec_r30h118;
using namespace flytec_r30h120;
using namespace flytec_r30h121;
// Owner-only coordination for one physical link. No driver or task access.
// All frame verification callbacks run on the application owner.
template<class Frame> class OwnerLink {
 AppRadio<Frame>& app_;
 HandshakeEndpoint<Frame> handshake_;
 SharedPriority& priority_;
 TxPriority appClass_=TxPriority::None;
 uint8_t epoch_=0;
 static uint8_t priorityBit(TxPriority p){return p==TxPriority::None?0:static_cast<uint8_t>(1U<<static_cast<unsigned>(p));}
 template<class F> static auto recoveryEcho(const F& f,uint16_t& echoed,int)->decltype(f.payload,bool()){
  return f.type==6U&&!f.flags&&!f.commandId&&
   flytec_rtos::recoveryProbeDecode(f.payload,flytec_rtos::RecoveryProbeKind::Echo,echoed)&&echoed!=0;
 }
 template<class F> static bool recoveryEcho(const F&,uint16_t&,long){return false;}
 void publish(){
  const bool appPending=app_.state()==TxState::Queued||app_.state()==TxState::Admitted;
  uint8_t mask=appPending?priorityBit(appClass_):0;
  if(app_.stopRetryPending())mask|=priorityBit(TxPriority::Stop);
  if(handshake_.needsService())mask|=priorityBit(TxPriority::Handshake);
  priority_.publish(mask);
 }
public:
 OwnerLink(AppRadio<Frame>& app,HandshakeLane<Frame>& lane,PhyEvents<Frame>& events,SharedPriority& priority):
  app_(app),handshake_(lane,events),priority_(priority){}
 // Session rollover deliberately preserves outstanding root-key CONFIRM.
 bool setSession(uint8_t epoch){
  epoch_=0;appClass_=TxPriority::None;
  const bool ok=app_.reset(epoch);
  if(ok)epoch_=epoch;
  publish();return ok;
 }
 void cancelHandshake(){handshake_.cancel();publish();}
 bool handshakePending()const{return handshake_.needsService();}
 bool beginHandshake(){
  if(handshake_.state()!=HandshakeTx::Idle)return false;
  const bool ok=handshake_.begin();publish();return ok;
 }
 template<class Policy,class Verify> bool submitHandshake(const Frame& f,uint32_t now,Policy policy,Verify verify){
  if(app_.stopRetryPending()||appClass_==TxPriority::Stop)return false;
  const bool ok=handshake_.submit(f,now,policy,verify);publish();return ok;
 }
 template<class Verify> bool submitApplication(const Frame& f,uint32_t now,TxPriority p,uint32_t age,Verify verify){
  if(!epoch_||f.sessionEpoch!=epoch_||!f.sequence||!age||p==TxPriority::Stop||p==TxPriority::Handshake||p==TxPriority::None||!verify(f))return false;
  if(app_.stopRetryPending()||!app_.submit(f,now,false,age))return false;
  appClass_=p;publish();return true;
 }
 // Boat-originated, authenticated feedback only. Never use this path for
 // a Remote command. Numeric types are the fixed radio protocol wire ABI.
 template<class Verify> bool submitFeedback(const Frame& f,uint32_t now,TxPriority p,uint32_t age,Verify verify){
  uint16_t echoedProbe=0;
  const bool taggedRecoveryEcho=p==TxPriority::Heartbeat&&recoveryEcho(f,echoedProbe,0);
  const bool shape=(f.type==1U&&p==TxPriority::Heartbeat)||
    taggedRecoveryEcho||
    ((f.type==5U||f.type==9U||f.type==15U)&&p==TxPriority::Mission)||
    (f.type==3U&&(p==TxPriority::Telemetry||p==TxPriority::Diagnostic));
  if(!shape||!epoch_||f.sessionEpoch!=epoch_||!f.sequence||!age||!verify(f)||!app_.feedbackReady())return false;
  if(!app_.submit(f,now,false,age,true,p==TxPriority::Mission?ReplyClass::Mission:ReplyClass::Routine))return false;
  appClass_=p;publish();return true;
 }
 // STOP-class reply may preempt queued ordinary work; never revokes active RF.
 // A false return can mean retained retry, not successful admission.
 template<class Verify> bool submitStop(const Frame& f,uint32_t now,uint32_t age,Verify verify){
  if(!epoch_||f.sessionEpoch!=epoch_||!f.sequence||!age||!verify(f))return false;
  priority_.publish(priorityBit(TxPriority::Stop));
  appClass_=TxPriority::Stop;
  const bool ok=app_.preemptStop(f,now,epoch_,age,ReplyClass::StopAck);publish();return ok;
 }
 // STOP feedback may use the reserved lane even while ordinary TX is
 // latched. Unlike a STOP ACK, it never preempts other queued work.
 template<class Verify> bool submitStatus(const Frame& f,uint32_t now,uint32_t age,bool stopped,Verify verify){
  if(!epoch_||f.sessionEpoch!=epoch_||!f.sequence||!age||!verify(f)||app_.stopRetryPending()||app_.terminalPending())return false;
  if(!app_.submit(f,now,stopped,age,true,ReplyClass::Status))return false;
  appClass_=stopped?TxPriority::Stop:TxPriority::Safety;publish();return true;
 }
 bool revokeStatus(const Ticket& t){
  const bool ok=app_.revokeCurrent(t,epoch_);
  if(ok){appClass_=TxPriority::None;publish();}
  return ok;
 }
 void poll(uint32_t now,uint32_t appTimeout,uint32_t handshakeTimeout){
  app_.poll();app_.expire(now,appTimeout,epoch_);handshake_.poll(now,handshakeTimeout);
  if(app_.stopRetryPending())app_.serviceStop(now);
  if(!app_.stopRetryPending()&&app_.state()!=TxState::Queued&&app_.state()!=TxState::Admitted)appClass_=TxPriority::None;
  publish();
 }
 bool rearm(bool authorized){
  if(!epoch_)return false;
  const bool ok=app_.rearm(epoch_,authorized);publish();return ok;
 }
 bool cancelDeferredStop(){
  const bool ok=app_.cancelStopRetry(epoch_);
  if(ok){appClass_=TxPriority::None;publish();}
  return ok;
 }
 bool ordinaryReady()const{return epoch_&&app_.ordinaryReady();}
 bool feedbackReady()const{return epoch_&&app_.feedbackReady();}
 bool stopRetryPending()const{return app_.stopRetryPending();}
 Ticket currentTicket()const{return app_.currentTicket();}
 bool takeApplication(Ticket& ticket,TxState& state){return app_.takeTerminal(ticket,state);}
 bool takeSuperseded(Ticket& ticket,TxState& state){return app_.takeSuperseded(ticket,state);}
 bool takeHandshake(Ticket& ticket,HandshakeTx& state){return handshake_.takeTerminal(ticket,state);}
 void close(){epoch_=0;appClass_=TxPriority::None;app_.reset(0);handshake_.close();priority_.publish(0);}
};
}

namespace flytec_r30h123 {
// Owner-side correlation of ordinary transmissions, separate from STOP and
// handshake services. One instance per link; queue acceptance is not success.
template<class Frame> class OrdinaryTx {
 Ticket ticket_{};
 Frame frame_{};
 uint32_t queuedAt_=0,age_=0;
 bool pending_=false;
public:
 bool pending()const{return pending_;}
 void reset(){pending_=false;}
 // Owner revokes an exact pending snapshot when transport trust is lost.
 // A PHY call already in progress is not interruptible by this operation.
 bool cancel(OwnerLink<Frame>& owner){
  if(!pending_)return false;
  owner.revokeStatus(ticket_);pending_=false;return true;
 }
 template<class Verify> bool submit(OwnerLink<Frame>& owner,const Frame& frame,uint32_t now,TxPriority priority,uint32_t age,Verify verify,bool outboundFeedback=false){
  if(pending_||!age)return false;
  const bool queued=outboundFeedback?owner.submitFeedback(frame,now,priority,age,verify):owner.submitApplication(frame,now,priority,age,verify);
  if(!queued)return false;
  ticket_=owner.currentTicket();frame_=frame;queuedAt_=now;age_=age;pending_=true;return true;
 }
 template<class Result> bool complete(const Ticket& ticket,TxState state,uint32_t now,uint8_t epoch,Result result){
  if(!pending_||!sameTicket(ticket,ticket_)||state==TxState::Idle||state==TxState::Queued||state==TxState::Admitted)return false;
  pending_=false;
  // Never refresh a heartbeat or clear an outbox from an obsolete epoch,
  // or from a completion that is already outside its original age budget.
  if(!epoch||epoch!=frame_.sessionEpoch||uint32_t(now-queuedAt_)>=age_)state=TxState::Uncertain;
  result(frame_,state,queuedAt_);return true;
 }
};
}

namespace flytec_r30h123 {
// Owner-side RX boundary. Capture time is local PHY observation, not sender
// tick or proof of RF age. The existing application must still enforce replay,
// command decoding and actuator policy. No RX callback runs on the PHY core.
template<class Frame> class OwnerRx {
 using Events=flytec_r30h118::PhyEvents<Frame>;
 flytec_r30h121::AppRadio<Frame>& app_;
 const uint8_t source_,destination_,flags_;
public:
 OwnerRx(flytec_r30h121::AppRadio<Frame>& app,uint8_t source,uint8_t destination,bool backup):
  app_(app),source_(source),destination_(destination),flags_(backup?1U:0U){}
 // Re-read session epoch per frame: an earlier handshake can roll the key.
 // Budget counts discarded frames as well as accepted frames.
 template<class Epoch,class RootVerify,class SessionVerify,class Dispatch>
 unsigned drain(uint32_t now,uint32_t age,Epoch epoch,RootVerify rootVerify,SessionVerify sessionVerify,Dispatch dispatch){
  unsigned delivered=0;
  typename Events::Rx rx{};
  for(unsigned i=0;i<4&&app_.receive(rx);++i){
   const auto& f=rx.frame;
   if(!age||uint32_t(now-rx.capturedAt)>=age||f.sourceId!=source_||f.destinationId!=destination_||f.flags!=flags_)continue;
   // Boat receives HELLO and PROOF only; reply types belong to the Remote.
   if(f.type==10U||f.type==12U){
    if(f.sessionEpoch||f.sequence||f.commandId||!rootVerify(f))continue;
   }else{
    const uint8_t active=epoch();
    if(!active||f.sessionEpoch!=active||!f.sequence)continue;
    if(f.type!=1U&&f.type!=2U&&f.type!=4U&&f.type!=8U&&f.type!=14U&&f.type!=16U)continue;
    if(!sessionVerify(f))continue;
   }
   dispatch(f,rx.capturedAt);++delivered;
  }
  return delivered;
 }
};
}

#include <stdint.h>
namespace flytec_r30h123 {
// Owner-core-only selection. Claim is irreversible for this object, even
// when initialization fails. Task exit alone does not grant legacy fallback.
enum class RadioOwner:uint8_t { Unclaimed,Legacy,Worker,Retired };
class RuntimeOwner {
 RadioOwner owner_=RadioOwner::Unclaimed;
public:
 RadioOwner mode()const{return owner_;}
 bool claimLegacy(){
  if(owner_!=RadioOwner::Unclaimed)return false;
  owner_=RadioOwner::Legacy;return true;
 }
 bool claimWorker(bool commissioned){
  if(!commissioned||owner_!=RadioOwner::Unclaimed)return false;
  owner_=RadioOwner::Worker;return true;
 }
 void retire(){if(owner_!=RadioOwner::Unclaimed)owner_=RadioOwner::Retired;}
 bool legacy()const{return owner_==RadioOwner::Legacy;}
 bool worker()const{return owner_==RadioOwner::Worker;}
};
}

#include <stdint.h>
#include <string.h>
namespace flytec_r30h123 {
// Latest validated snapshots only. No STOP STATUS or diagnostic bypass.
// Latitude/longitude are paired by sample ID AND snapshot time. If a new
// pair replaces an incomplete transmission, abandon the older remainder.
class TelemetrySlots {
 struct Slot {uint8_t bytes[8]{};uint32_t at=0;bool valid=false;};
 Slot slots_[6]{};
 uint8_t cursor_=0;
 bool coordinateHalfTaken_=false;
 uint8_t coordinateId_=0;
 uint32_t coordinateAt_=0;
 bool matches(const Slot& a,const Slot& b)const{
  return a.valid&&b.valid&&a.at==b.at&&a.bytes[1]==b.bytes[1];
 }
public:
 static constexpr uint32_t MAX_AGE_MS=250U;
 void reset(){for(auto &s:slots_)s.valid=false;cursor_=0;coordinateHalfTaken_=false;}
 bool offer(const uint8_t p[8],uint32_t at){
  if(!p||p[0]<2||p[0]>7)return false;
  const unsigned idx=p[0]-2;
  if(idx==1||idx==2){
   if(coordinateHalfTaken_&&(coordinateAt_!=at||coordinateId_!=p[1])){
    slots_[1].valid=slots_[2].valid=false;coordinateHalfTaken_=false;
   }
   auto &other=slots_[idx==1?2:1];
   if(other.valid&&(other.at!=at||other.bytes[1]!=p[1]))other.valid=false;
  }
  auto &s=slots_[idx];memcpy(s.bytes,p,8);s.at=at;s.valid=true;return true;
 }
 bool take(uint32_t now,uint8_t p[8],uint32_t &remaining){
  if(!p)return false;
  for(unsigned i=0;i<6;++i){
   const unsigned idx=(cursor_+i)%6;auto &s=slots_[idx];
   if(!s.valid)continue;
   const uint32_t age=uint32_t(now-s.at);
   if(age>=MAX_AGE_MS){s.valid=false;if(idx==1||idx==2)coordinateHalfTaken_=false;continue;}
   if(idx==1||idx==2){
    const auto &other=slots_[idx==1?2:1];
    const bool remainder=coordinateHalfTaken_&&coordinateAt_==s.at&&coordinateId_==s.bytes[1];
    if(!remainder&&!matches(s,other))continue;
    coordinateHalfTaken_=!remainder;coordinateAt_=s.at;coordinateId_=s.bytes[1];
   }
   s.valid=false;memcpy(p,s.bytes,8);remaining=MAX_AGE_MS-age;cursor_=(idx+1)%6;return true;
  }
  return false;
 }
};
}
