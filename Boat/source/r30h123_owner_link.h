#pragma once
#include "r30h121_app_radio.h"
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
 static uint8_t bit(TxPriority p){return p==TxPriority::None?0:static_cast<uint8_t>(1U<<static_cast<unsigned>(p));}
 void publish(){
  const bool appPending=app_.state()==TxState::Queued||app_.state()==TxState::Admitted;
  uint8_t mask=appPending?bit(appClass_):0;
  if(app_.stopRetryPending())mask|=bit(TxPriority::Stop);
  if(handshake_.state()==HandshakeTx::Pending)mask|=bit(TxPriority::Handshake);
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
 // STOP-class reply may preempt queued ordinary work; never revokes active RF.
 // A false return can mean retained retry, not successful admission.
 template<class Verify> bool submitStop(const Frame& f,uint32_t now,uint32_t age,Verify verify){
  if(!epoch_||f.sessionEpoch!=epoch_||!f.sequence||!age||!verify(f))return false;
  priority_.publish(bit(TxPriority::Stop));
  appClass_=TxPriority::Stop;
  const bool ok=app_.preemptStop(f,now,epoch_,age);publish();return ok;
 }
 void poll(uint32_t now,uint32_t appTimeout,uint32_t handshakeTimeout){
  app_.poll();app_.expire(now,appTimeout);handshake_.poll(now,handshakeTimeout);
  if(app_.stopRetryPending())app_.serviceStop(now);
  if(!app_.stopRetryPending()&&app_.state()!=TxState::Queued&&app_.state()!=TxState::Admitted)appClass_=TxPriority::None;
  publish();
 }
 bool rearm(bool authorized){
  if(!epoch_)return false;
  const bool ok=app_.rearm(epoch_,authorized);publish();return ok;
 }
 bool takeApplication(Ticket& ticket,TxState& state){return app_.takeTerminal(ticket,state);}
 bool takeSuperseded(Ticket& ticket,TxState& state){return app_.takeSuperseded(ticket,state);}
 bool takeHandshake(Ticket& ticket,HandshakeTx& state){return handshake_.takeTerminal(ticket,state);}
 void close(){epoch_=0;appClass_=TxPriority::None;app_.reset(0);handshake_.close();priority_.publish(0);}
};
}
