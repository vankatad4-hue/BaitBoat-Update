#pragma once
#include "remote_ordinary_intent.h"
#include "radio_protocol.h"
// Experimental producer only. The application must own all calls, run the
// critical scheduler first, and invalidate transport on lost authority.
// No automatic rearm, backup, retry, peer ACK or mission submission.
namespace flytec_rtos {
template<class Tx> class RemoteAsyncOrdinary {
public:
 explicit RemoteAsyncOrdinary(Tx& tx):tx_(tx){}
 OrdinaryIntent& intent(){return intent_;}
 // Return false on in-flight authority loss or transport fault; caller must
 // revoke the ticket/coordinate reset, not simply discard the ledger.
 template<class Encode> bool service(const OrdinaryAuthorization& auth,uint32_t now,
   bool criticalGap,uint8_t epoch,uint16_t& sequence,uint16_t rekeyAt,const uint8_t key[32],Encode encode){
  using S=OrdinaryIntent::State;using Link=typename Tx::Link;using Submit=typename Tx::Submit;
  if(intent_.state()==S::InFlight)return intent_.stillAllowed(auth,now);
  if(intent_.state()!=S::Staged)return true;
  if(!intent_.stillAllowed(auth,now)){intent_.discardStaged();return true;}
  if(!criticalGap||tx_.pending(Link::Primary))return true;
  if(!epoch||!key||rekeyAt<2||sequence>=uint16_t(rekeyAt-1)||tx_.linkFault(Link::Primary))return false;
  BoatCommand cmd=intent_.command();cmd.sequence=uint16_t(sequence+1);
  RadioFrame f{};
  if(!encode(cmd,1,2,epoch,f,key))return false;
  if(f.sourceId!=1||f.destinationId!=2||f.flags||f.sessionEpoch!=epoch||f.sequence!=cmd.sequence||
     f.commandId!=cmd.commandId||f.payload[0]!=cmd.type||
     f.type!=(cmd.type==CMD_MANUAL_MOTORS?RADIO_MSG_CONTROL:RADIO_MSG_ACTION))return false;
  sequence=uint16_t(cmd.sequence);uint32_t ticket=0;
  const auto result=tx_.submit(Link::Primary,f,cmd.createdAtMs,ordinaryLifetime(cmd),now,ticket);
  if(result==Submit::Busy)return true;
  if(result==Submit::Denied||result==Submit::Expired){intent_.discardStaged();return true;}
  if(result!=Submit::Queued)return false;
  return intent_.submitted(ticket,tx_.generation(),sequence,epoch);
 }
 bool observeCompletion(const typename Tx::Result& c,uint32_t now){
  using Link=typename Tx::Link;using Kind=typename Tx::Kind;using Outcome=typename Tx::Outcome;
  if(c.link!=Link::Primary||c.kind!=Kind::Ordinary||c.stop||
     c.messageType!=(intent_.command().type==CMD_MANUAL_MOTORS?RADIO_MSG_CONTROL:RADIO_MSG_ACTION))return false;
  return intent_.complete(c.ticket,c.generation,c.commandId,c.sequence,c.epoch,c.createdMs,c.lifetimeMs,
    c.outcome==Outcome::NrfAck,now);
 }
private:
 Tx& tx_;OrdinaryIntent intent_;
};
}
